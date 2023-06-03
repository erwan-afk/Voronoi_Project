#include "application_ui.h"
#include "SDL2_gfxPrimitives.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>

#define EPSILON 0.0001f

struct Coords
{
    int x, y;
    float distance;

     bool operator==(const Coords& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct Segment
{
    Coords p1, p2;
};

struct Triangle
{
    Coords p1, p2, p3;
    bool complet=false;
};

struct Polygone {
    std::vector<Coords> points;
};



// Définir un type pour représenter un voisin
// supposez que Coords est défini comme ça:

// supposez que vos données sont définies comme suit:

struct Application
{
    int width, height;
    Coords focus{100, 100};

    std::vector<Coords> points;
    std::vector<Triangle> triangles;
    std::vector<Polygone> polygones;
    std::vector<Coords> circumCenters;
    std::vector<Segment> segment_voronoi;
};

bool compareCoords(Coords point1, Coords point2)
{
    if (point1.y == point2.y)
        return point1.x < point2.x;
    return point1.y < point2.y;
}

void drawPoints(SDL_Renderer *renderer, const std::vector<Coords> &points,int r,int g,int b)
{
    for (std::size_t i = 0; i < points.size(); i++)
    {
        filledCircleRGBA(renderer, points[i].x, points[i].y, 3, r, g, b, SDL_ALPHA_OPAQUE);
    }
}



void drawSegments(SDL_Renderer *renderer, const std::vector<Segment> &segments)
{
    for (std::size_t i = 0; i < segments.size(); i++)
    {
        lineRGBA(
            renderer,
            segments[i].p1.x, segments[i].p1.y,
            segments[i].p2.x, segments[i].p2.y,
            255, 0, 0, SDL_ALPHA_OPAQUE);
    }
}

void drawTriangles(SDL_Renderer *renderer, const std::vector<Triangle> &triangles)
{
    for (std::size_t i = 0; i < triangles.size(); i++)
    {
        const Triangle& t = triangles[i];
        trigonRGBA(
            renderer,
            t.p1.x, t.p1.y,
            t.p2.x, t.p2.y,
            t.p3.x, t.p3.y,
            0, 0, 0, SDL_ALPHA_OPAQUE
        );
    }
}



void drawPolygons(SDL_Renderer *renderer, const std::vector<Polygone> &polygons)
{
    for (const auto& polygon : polygons)
    {
        int numPoints = polygon.points.size();
        std::vector<Sint16> vx(numPoints), vy(numPoints);

        for (int i = 0; i < numPoints; ++i)
        {
            vx[i] = polygon.points[i].x;
            vy[i] = polygon.points[i].y;
        }

        filledPolygonRGBA(
            renderer,
            vx.data(),
            vy.data(),
            numPoints,
            0, 0, 0, SDL_ALPHA_OPAQUE
        );

        // Dessiner les points des centres des triangles en rouge
        for (int i = 0; i < numPoints; ++i)
        {
            filledCircleRGBA(
                renderer,
                vx[i],
                vy[i],
                3,
                255, 0, 0, SDL_ALPHA_OPAQUE
            );
        }
    }
}




void draw(SDL_Renderer *renderer, const Application &app)
{
    /* Remplissez cette fonction pour faire l'affichage du jeu */
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    drawPoints(renderer, app.points,0,0,0);
    drawTriangles(renderer, app.triangles);
    drawPoints(renderer,app.circumCenters,255,0,0);
    drawSegments(renderer, app.segment_voronoi);
    
}




/*
   Détermine si un point se trouve dans un cercle définit par trois points
   Retourne, par les paramètres, le centre et le rayon
*/
bool CircumCircle(
    float pX, float pY,
    float x1, float y1, float x2, float y2, float x3, float y3,
    float *xc, float *yc, float *rsqr
)
{
    float m1, m2, mx1, mx2, my1, my2;
    float dx, dy, drsqr;
    float fabsy1y2 = fabs(y1 - y2);
    float fabsy2y3 = fabs(y2 - y3);

    /* Check for coincident points */
    if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
        return (false);

    if (fabsy1y2 < EPSILON)
    {
        m2 = -(x3 - x2) / (y3 - y2);
        mx2 = (x2 + x3) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (x2 + x1) / 2.0;
        *yc = m2 * (*xc - mx2) + my2;
    }
    else if (fabsy2y3 < EPSILON)
    {
        m1 = -(x2 - x1) / (y2 - y1);
        mx1 = (x1 + x2) / 2.0;
        my1 = (y1 + y2) / 2.0;
        *xc = (x3 + x2) / 2.0;
        *yc = m1 * (*xc - mx1) + my1;
    }
    else
    {
        m1 = -(x2 - x1) / (y2 - y1);
        m2 = -(x3 - x2) / (y3 - y2);
        mx1 = (x1 + x2) / 2.0;
        mx2 = (x2 + x3) / 2.0;
        my1 = (y1 + y2) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        if (fabsy1y2 > fabsy2y3)
        {
            *yc = m1 * (*xc - mx1) + my1;
        }
        else
        {
            *yc = m2 * (*xc - mx2) + my2;
        }
    }

    dx = x2 - *xc;
    dy = y2 - *yc;
    *rsqr = dx * dx + dy * dy;

    dx = pX - *xc;
    dy = pY - *yc;
    drsqr = dx * dx + dy * dy;

    return ((drsqr - *rsqr) <= EPSILON ? true : false);
}


void construitVoronoi(Application &app) {
    // Trier les points selon x
    std::sort(app.points.begin(), app.points.end(), [](const Coords& a, const Coords& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    // Vider la liste existante de triangles
    app.triangles.clear();
    app.circumCenters.clear();
    app.segment_voronoi.clear();

    // Créer un très grand triangle
    Triangle largeTriangle {{-1000, -1000}, {500, 3000}, {1500, -1000}};

    // Ajouter le grand triangle à la liste
    app.triangles.push_back(largeTriangle);

    // Pour chaque point
    for (auto& P : app.points) {
        std::vector<Segment> LS;  // Liste des segments
        
        auto it = app.triangles.begin();
        while (it != app.triangles.end()) {
            float xc, yc, rsqr;
            bool inCircumcircle = CircumCircle(P.x, P.y, it->p1.x, it->p1.y, it->p2.x, it->p2.y, it->p3.x, it->p3.y, &xc, &yc, &rsqr);
            

            if (inCircumcircle) {
                LS.push_back({it->p1, it->p2});
                LS.push_back({it->p2, it->p3});
                LS.push_back({it->p3, it->p1});

                it = app.triangles.erase(it);  // Enlever le triangle
            } else {
                ++it;
            }
        }

        // Pour chaque segment dans LS
        for (auto it = LS.begin(); it != LS.end(); ) {
            bool isDuplicated = false;

            // Vérifier s'il existe un doublon
            for (auto jt = LS.begin(); jt != LS.end(); ++jt) {
                if (jt != it && ((it->p1 == jt->p2 && it->p2 == jt->p1) || (it->p1 == jt->p1 && it->p2 == jt->p2))) {
                    isDuplicated = true;
                    LS.erase(jt);
                    break;
                }
            }

            if (isDuplicated) {
                it = LS.erase(it);
            } else {
                ++it;
            }
        }

        // Pour chaque segment restant
        for (auto& S : LS) {
            // Créer un nouveau triangle avec le point et le segment
            Triangle newTriangle {S.p1, S.p2, P};

                // Calculer le centre du cercle circonscrit

            // Ajouter le nouveau triangle à la liste de triangles
            app.triangles.push_back(newTriangle);
     
        }

        
    }



    
        for (int i = 0; i < app.triangles.size(); i++)
    {
            Triangle newTriangle = app.triangles[i];
                                // Calculer le centre du cercle circonscrit
            float xce, yce, rsqre;
            bool oo = CircumCircle(newTriangle.p1.x,newTriangle.p1.y ,newTriangle.p1.x, newTriangle.p1.y, newTriangle.p2.x, newTriangle.p2.y, newTriangle.p3.x, newTriangle.p3.y, &xce, &yce, &rsqre);

            app.circumCenters.push_back({static_cast<int>(xce), static_cast<int>(yce)});
        
    }

    
    

    



    for (size_t i = 0; i < app.circumCenters.size(); ++i) {
        for (size_t j = i + 1; j < app.circumCenters.size(); ++j) {

            if (i!= j)
            {

                 // Vérifier si les triangles i et j sont adjacents
            int commonPointsCount = 0;
            if (app.triangles[i].p1 == app.triangles[j].p1 || app.triangles[i].p1 == app.triangles[j].p2 || app.triangles[i].p1 == app.triangles[j].p3) {
                commonPointsCount++;
            }
            if (app.triangles[i].p2 == app.triangles[j].p1 || app.triangles[i].p2 == app.triangles[j].p2 || app.triangles[i].p2 == app.triangles[j].p3) {
                commonPointsCount++;
            }
            if (app.triangles[i].p3 == app.triangles[j].p1 || app.triangles[i].p3 == app.triangles[j].p2 || app.triangles[i].p3 == app.triangles[j].p3) {
                commonPointsCount++;
            }

            if (commonPointsCount == 2) {
                // Relier les triangles i et j en ajoutant le segment entre leurs centres
                Segment newSegment{app.circumCenters[i], app.circumCenters[j]};
                app.segment_voronoi.push_back(newSegment);
            }
               
            }
            

           
        }
    }

}




bool handleEvent(Application &app)
{
    /* Remplissez cette fonction pour gérer les inputs utilisateurs */
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return false;
        else if (e.type == SDL_WINDOWEVENT_RESIZED)
        {
            app.width = e.window.data1;
            app.height = e.window.data1;
        }
        else if (e.type == SDL_MOUSEWHEEL)
        {
        }
        else if (e.type == SDL_MOUSEBUTTONUP)
        {
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                app.focus.x = e.button.x;
                app.focus.y = e.button.y;
                app.points.clear();
                app.triangles.clear();
            }
            else if (e.button.button == SDL_BUTTON_LEFT)
            {
                app.focus.y = 0;
                app.points.push_back(Coords{e.button.x, e.button.y});
                construitVoronoi(app);
            }
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    SDL_Window *gWindow;
    SDL_Renderer *renderer;
    Application app{720, 720, Coords{0, 0}};
    bool is_running = true;

    // Creation de la fenetre
    gWindow = init("Awesome Voronoi", 720, 720);

    if (!gWindow)
    {
        SDL_Log("Failed to initialize!\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(gWindow, -1, 0); // SDL_RENDERER_PRESENTVSYNC

    /*  GAME LOOP  */
    while (true)
    {
        // INPUTS
        is_running = handleEvent(app);
        if (!is_running)
            break;

        // EFFACAGE FRAME
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // DESSIN
        draw(renderer, app);

        // VALIDATION FRAME
        SDL_RenderPresent(renderer);

        // PAUSE en ms
        SDL_Delay(1000 / 30);
    }

    // Free resources and close SDL
    close(gWindow, renderer);

    return 0;
}


















