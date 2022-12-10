const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float PRECISION = 0.001;
const vec3 BG_COLOR = vec3(0.6);


struct Light {
    vec3 position;
    vec4 color;
};

struct Material{
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

struct Surface {
    float sd; // signed distance
    vec3 col; // color
};

// Definition of all curves
struct Sphere {
    vec3 center;
    float radius;
    vec3 col;
};

Surface sdSphere(vec3 p, const Sphere sphere) {
    return Surface(length(p - sphere.center) - sphere.radius, sphere.col);
}

Surface minSurf(Surface a, Surface b) {
    if (a.sd < b.sd) {
        return a;
    } else {
        return b;
    }
}


Sphere sphere1 = Sphere(vec3(-2.5, 0, -2), 1.0, vec3(1, 0.58, 0.29));
Sphere sphere2 = Sphere(vec3(2.5, 0, -2), 0.5, vec3(0, 0.8, 0.8));
Surface sdScene(vec3 p) {
    Surface val = sdSphere(p, sphere1);
    val = minSurf(val, sdSphere(p, sphere2));
    return val;
}


// Ray Marching Definition
Surface rayMarch(vec3 ro, vec3 rd, float start, float end) {
    float depth = start;
    Surface co; // closest object

    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 p = ro + depth * rd;
        co = sdScene(p);
        depth += co.sd;
        if (co.sd < PRECISION || depth > end) break;
    }

    co.sd = depth;

    return co;
}


vec3 calcNormal(vec3 p) {
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
            e.xyy * sdScene(p + e.xyy).sd +
            e.yyx * sdScene(p + e.yyx).sd +
            e.yxy * sdScene(p + e.yxy).sd +
            e.xxx * sdScene(p + e.xxx).sd);
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    vec3 col = vec3(0);

    // define the position of the camera
    vec3 ro = vec3(0, 0, 2);
    vec3 rd = normalize(vec3(uv, -1));

    Surface co = rayMarch(ro, rd, MIN_DIST, MAX_DIST);
    vec3 lightPosition = vec3(2, 2, 4);

    if (co.sd > 100.0) {
        // ray didn't hit anything
        col = BG_COLOR;
    } else {
        // ray hit something
        // 我们必须先计算光射在曲面的点，才能算法向量
        vec3 p = ro + rd * co.sd;
        vec3 normal = calcNormal(p);
        vec3 lightDirection = normalize(lightPosition - p);
        float dif = clamp(dot(normal, lightDirection), 0.1, 1.);

        col = vec3(dif) * co.col;
    }

    // Output to screen
    fragColor = vec4(col, 1.0);
}
