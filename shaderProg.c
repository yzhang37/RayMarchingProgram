const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float PRECISION = 0.001;
const float AMBIENT = 0.05;
const vec3 BG_COLOR = vec3(AMBIENT);


struct Light {
    vec3 position;
    vec4 color;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float highlight;
};

struct Camera {
    vec3 position;
    vec3 lookAt;
    vec3 up;
    float fov;
};

// Definition of all curves
struct Sphere {
    vec3 center;
    float radius;
    vec3 col;
    Material mat;
};

float sdSphere(vec3 p, const Sphere sphere) {
    return length(p - sphere.center) - sphere.radius;
}

struct RoundBox {
    vec3 center;
    vec3 size;
    float radius;
    vec3 col;
    Material mat;
};

float sdRoundBox(vec3 p, const RoundBox round)
{
    vec3 q = abs(p - round.center) - round.size;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - round.radius;
}

struct Plane {
    float y_offset;
    Material mat;
};

float sdPlane(vec3 p, const Plane plane) {
    return p.y - plane.y_offset;
}

// build the scene
const Material mat_round = Material(BG_COLOR, vec3(0.6), vec3(0.15), 32.);
RoundBox round1 = RoundBox(vec3(-2., 0., 0.5), vec3(0.5, 0.3, 0.3), 0.2, vec3(1., 0., 0.), mat_round);
RoundBox round2 = RoundBox(vec3(2., 0., -0.5), vec3(0.5, 0.3, 0.3), 0.2, vec3(0., 1., 0.), mat_round);
const Material mat_sph = Material(BG_COLOR, vec3(0.3), vec3(0.85), 16.);
Sphere sphere1 = Sphere(vec3(0, 0, 0), 0.55, vec3(1, 0.58, 0.29), mat_sph);
const Material mat_plan = Material(vec3(0.0), vec3(0.5), vec3(0.5), 64.);
Plane plane1 = Plane(-1., mat_plan);

vec2 opU(vec2 d1, vec2 d2) {
    return (d1.x < d2.x) ? d1 : d2;
}

vec2 map(vec3 p) {
    vec2 res = vec2(MAX_DIST, 0);
    res = opU(res, vec2(sdRoundBox(p, round1), 1));
    res = opU(res, vec2(sdRoundBox(p, round2), 2));
    res = opU(res, vec2(sdSphere(p, sphere1), 3));
    res = opU(res, vec2(sdPlane(p, plane1), 4));
    return res;
}


// Ray Marching Definition
vec2 rayMarch(vec3 ro, vec3 rd, float start, float end) {
    float depth = start;
    vec2 res = vec2(0.0);
    float id = 0.;

    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 p = ro + depth * rd;
        res = map(p);
        depth += res.x;
        id = res.y;
        if (res.x < PRECISION || depth > end) break;
    }
    return vec2(depth, id);
}


vec3 calcNormal(vec3 p) {
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
            e.xyy * map(p + e.xyy).x +
            e.yyx * map(p + e.yyx).x +
            e.yxy * map(p + e.yxy).x +
            e.xxx * map(p + e.xxx).x);
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    vec3 col = vec3(0);

    // define the position of the camera
    vec3 ro = vec3(0, 0, 5);
    vec3 rd = normalize(vec3(uv, -1));

    vec2 res = rayMarch(ro, rd, MIN_DIST, MAX_DIST);
    vec3 lightPosition = vec3(2, 2, 4);

    if (res.x > 100.0) {
        // ray didn't hit anything
        col = BG_COLOR;
    } else {
        // ray hit something
        vec3 p = ro + rd * res.x;
        vec3 normal = calcNormal(p);
        vec3 lightDirection = normalize(lightPosition - p);
        int id = int(res.y);

        // first compute illumination
        Material mat;
        if (id == 1)
            mat = round1.mat;
        else if (id == 2)
            mat = round2.mat;
        else if (id == 3)
            mat = sphere1.mat;
        else if (id == 4)
            mat = plane1.mat;
        else
            mat = Material(BG_COLOR, vec3(0.0), vec3(0.0), 2.);

        // ambient
        vec3 il = mat.ambient;
        // diffuse
        vec3 dif = clamp(dot(normal, lightDirection), 0., 1.) * mat.diffuse;
        il += clamp(dif, mat.ambient, vec3(1.));
        // specular
        il += mat.specular * pow(clamp(dot(normal, reflect(-lightDirection, normal)), 0., 1.), mat.highlight);
        il = clamp(il, 0., 1.);

        // then compute the color
        if (id == 1) {
            col = il * round1.col;
        } else if (id == 2) {
            col = il * round2.col;
        } else if (id == 3) {
            col = il * sphere1.col;
        } else if (id == 4) {
            col = il * vec3(1. + 0.7 * mod(floor(p.x) + floor(p.z), 2.0));
        } else {
            col = il;
        }
    }

    // Output to screen
    fragColor = vec4(col, 1.0);
}
