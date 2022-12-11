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
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
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
};

float sdSphere(vec3 p, const Sphere sphere) {
    return length(p - sphere.center) - sphere.radius;
}

float sdFloor(vec3 p) {
    return p.y + 1.; // y = -1
}

// build the scene
Sphere sphere1 = Sphere(vec3(-2.5, 0, -2), 1.0, vec3(1, 0.58, 0.29));
Sphere sphere2 = Sphere(vec3(2.5, 0, -2), 0.5, vec3(0, 0.8, 0.8));

vec2 opU(vec2 d1, vec2 d2) {
    return (d1.x < d2.x) ? d1 : d2;
}

vec2 map(vec3 p) {
    vec2 res = vec2(MAX_DIST, 0);
    res = opU(res, vec2(sdSphere(p, sphere1), 1));
    res = opU(res, vec2(sdSphere(p, sphere2), 2));
    res = opU(res, vec2(sdFloor(p), 3));
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
        float dif = clamp(dot(normal, lightDirection), AMBIENT, 1.);

        int id = int(res.y);
        if (id == 1) {
            col = dif * sphere1.col;
        } else if (id == 2) {
            col = dif * sphere2.col;
        } else if (id == 3) {
            col = dif * vec3(1. + 0.7 * mod(floor(p.x) + floor(p.z), 2.0));
        } else {
            col = vec3(dif);
        }
    }

    // Output to screen
    fragColor = vec4(col, 1.0);
}
