const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float PRECISION = 0.001;
const float sphere_radius = 1.0;


float sdSphere(vec3 p, float r) {
    vec3 offset = vec3(0, 0, -2);
    return length(p - offset) - r;
}


float rayMarch(vec3 ro, vec3 rd, float start, float end) {
    float depth = start;

    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 p = ro + depth * rd;
        float d = sdSphere(p, sphere_radius);
        depth += d;
        if (d < PRECISION || depth > end) break;
    }

    return depth;
}


// TODO: 把这里改成 Swizzling 的写法
vec3 calcNormal(vec3 p) {
    float e = 0.0005;
    return normalize(vec3(
            sdSphere(vec3(p.x + e, p.y, p.z), sphere_radius) - sdSphere(vec3(p.x - e, p.y, p.z), sphere_radius),
            sdSphere(vec3(p.x, p.y + e, p.z), sphere_radius) - sdSphere(vec3(p.x, p.y - e, p.z), sphere_radius),
            sdSphere(vec3(p.x, p.y, p.z + e), sphere_radius) - sdSphere(vec3(p.x, p.y, p.z - e), sphere_radius)
    ));
}


vec3 lightPosition = vec3(2, 2, 4);


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    vec3 col = vec3(0);

    // define the position of the camera
    vec3 ro = vec3(0, 0, 2);
    vec3 rd = normalize(vec3(uv, -1));

    float d = rayMarch(ro, rd, MIN_DIST, MAX_DIST);

    if (d > 100.0) {
        // ray didn't hit anything
        col = vec3(0.6);
    } else {
        // ray hit something
        // 我们必须先计算光射在曲面的点，才能算法向量
        vec3 p = ro + rd * d;
        vec3 normal = calcNormal(p);
        vec3 lightDirection = normalize(lightPosition - p);
        float dif = clamp(dot(normal, lightDirection), 0.1, 1.);

        col = vec3(dif) * vec3(1, 0.58, 0.29);;
    }

    // Output to screen
    fragColor = vec4(col, 1.0);
}
