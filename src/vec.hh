#ifndef VEC_HH
#define VEC_HH

typedef struct {
    double x,y;
} vec;

inline double dot(vec &a, vec &b) {
    return a.x*b.x+a.y*b.y;
}

inline double mag(vec &a) {
    return sqrt(a.x*a.x+a.y*a.y);
}

inline vec sub(vec &a, vec &b) {
    vec res;
    res.x = a.x-b.x;
    res.y = a.y-b.y;
    return res;
}

inline double dist(vec &a, vec &b) {
    vec res = sub(a,b);
    return mag(res);
}

inline double copy(vec &src, vec &dest) {
    dest.x = src.x;
    dest.y = src.y;
}

inline double scale(vec &a, double b) {
    a.x *= b;
    a.y *= b;
}

inline double unit(vec &a) {
    scale(a,1.0/mag(a));
}

inline void rotate(vec &a, double rad) {
    double x = a.x;
    a.x = cos(rad)*x-sin(rad)*a.y;
    a.y = sin(rad)*x+cos(rad)*a.y;
}

inline void linadd(vec &a, vec &b, double scale) {
    a.x += b.x*scale;
    a.y += b.y*scale;
}

#endif
