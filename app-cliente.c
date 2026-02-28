// Test (solo llama a API local)

#include <stdio.h> // Para printf
#include <string.h> // Para strcmp, memset

#include "claves.h" // Prototipos de la API y definición de struct Paquete

// Macro para comparar enteros y mostrar mensaje de error con los valores obtenidos y esperados
#define CHECK_EQ_INT(msg, got, exp) do { \
    if ((got) != (exp)) { \
        printf("[FAIL] %s | got=%d expected=%d\n", (msg), (got), (exp)); \
        return 1; \
    } else { \
        printf("[ OK ] %s | %d\n", (msg), (got)); \
    } \
} while(0)

// Macro para comparar strings y mostrar mensaje de error con los valores obtenidos y esperados
#define CHECK_EQ_STR(msg, got, exp) do { \
    if (strcmp((got), (exp)) != 0) { \
        printf("[FAIL] %s | got=\"%s\" expected=\"%s\"\n", (msg), (got), (exp)); \
        return 1; \
    } else { \
        printf("[ OK ] %s | \"%s\"\n", (msg), (got)); \
    } \
} while(0)

// Función auxiliar para comparar dos vectores de float
static int check_float_vec(const float *a, const float *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

// Batería de pruebas para la API local (claves.c)
int main(void) {
    printf("=== BATERIA DE PRUEBAS (local) ===\n");

    // 0) Estado inicial limpio
    int r = destroy();
    CHECK_EQ_INT("destroy() inicial", r, 0);

    // 1) exist() en clave inexistente -> 0
    r = exist("k1");
    CHECK_EQ_INT("exist(\"k1\") inexistente", r, 0);

    // 2) get_value() en inexistente -> -1
    char out_v1[256];
    int out_N = 0;
    float out_v2[32];
    struct Paquete out_p;

    r = get_value("k1", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value(\"k1\") inexistente", r, -1);

    // 3) set_value() OK
    struct Paquete p1 = {1, 2, 3};
    float v2_1[3] = {1.0f, 2.0f, 3.0f};

    r = set_value("k1", "valor1", 3, v2_1, p1);
    CHECK_EQ_INT("set_value(\"k1\") OK", r, 0);

    // 4) set_value() duplicada -> -1
    r = set_value("k1", "otro", 3, v2_1, p1);
    CHECK_EQ_INT("set_value(\"k1\") duplicada", r, -1);

    // 5) exist() ahora -> 1
    r = exist("k1");
    CHECK_EQ_INT("exist(\"k1\") existente", r, 1);

    // 6) set_value() con N fuera de rango -> -1
    float v2_dummy[32];
    for (int i = 0; i < 32; i++) v2_dummy[i] = (float)i;

    r = set_value("k2", "v", 0, v2_dummy, p1);
    CHECK_EQ_INT("set_value N=0 (fuera de rango)", r, -1);

    r = set_value("k2", "v", 33, v2_dummy, p1);
    CHECK_EQ_INT("set_value N=33 (fuera de rango)", r, -1);

    // 7) get_value() OK y comprobación de contenido
    memset(out_v1, 0, sizeof(out_v1));
    out_N = 0;
    memset(out_v2, 0, sizeof(out_v2));
    out_p.x = out_p.y = out_p.z = 0;

    r = get_value("k1", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value(\"k1\") OK", r, 0);
    CHECK_EQ_STR("value1 correcto", out_v1, "valor1");
    CHECK_EQ_INT("N correcto", out_N, 3);
    if (!check_float_vec(out_v2, v2_1, 3)) {
        printf("[FAIL] V_value2 no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 coincide\n");
    }
    CHECK_EQ_INT("Paquete.x", out_p.x, 1);
    CHECK_EQ_INT("Paquete.y", out_p.y, 2);
    CHECK_EQ_INT("Paquete.z", out_p.z, 3);

    // 8) modify_value() inexistente -> -1
    struct Paquete p2 = {9, 8, 7};
    float v2_2[2] = {10.0f, 20.0f};
    r = modify_value("k_no", "x", 2, v2_2, p2);
    CHECK_EQ_INT("modify_value clave inexistente", r, -1);

    // 9) modify_value() OK
    r = modify_value("k1", "valor_mod", 2, v2_2, p2);
    CHECK_EQ_INT("modify_value(\"k1\") OK", r, 0);

    // 10) get_value() tras modify_value
    r = get_value("k1", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value(\"k1\") tras modify", r, 0);
    CHECK_EQ_STR("value1 tras modify", out_v1, "valor_mod");
    CHECK_EQ_INT("N tras modify", out_N, 2);
    if (!check_float_vec(out_v2, v2_2, 2)) {
        printf("[FAIL] V_value2 tras modify no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 tras modify coincide\n");
    }
    CHECK_EQ_INT("Paquete.x tras modify", out_p.x, 9);
    CHECK_EQ_INT("Paquete.y tras modify", out_p.y, 8);
    CHECK_EQ_INT("Paquete.z tras modify", out_p.z, 7);

    // 11) delete_key() inexistente -> -1
    r = delete_key("k_no");
    CHECK_EQ_INT("delete_key clave inexistente", r, -1);

    // 12) delete_key() OK
    r = delete_key("k1");
    CHECK_EQ_INT("delete_key(\"k1\") OK", r, 0);

    // 13) exist() después de borrar -> 0
    r = exist("k1");
    CHECK_EQ_INT("exist(\"k1\") tras delete", r, 0);

    // 14) destroy() final -> 0
    r = destroy();
    CHECK_EQ_INT("destroy() final", r, 0);

    // 15) set_value() con N=1 (limite inferior) -> OK
    struct Paquete p_min = {4, 5, 6};
    float v2_min[1] = {42.0f};
    r = set_value("k_min", "v_min", 1, v2_min, p_min);
    CHECK_EQ_INT("set_value k_min N=1", r, 0);

    // 16) get_value() para k_min y comprobacion
    memset(out_v1, 0, sizeof(out_v1));
    out_N = 0;
    memset(out_v2, 0, sizeof(out_v2));
    out_p.x = out_p.y = out_p.z = 0;
    r = get_value("k_min", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value k_min OK", r, 0);
    CHECK_EQ_STR("value k_min correcto", out_v1, "v_min");
    CHECK_EQ_INT("N k_min correcto", out_N, 1);
    if (!check_float_vec(out_v2, v2_min, 1)) {
        printf("[FAIL] V_value2 k_min no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 k_min coincide\n");
    }

    // 17) set_value() con N=32 (limite superior) -> OK
    struct Paquete p_max = {7,8,9};
    float v2_max[32];
    for (int i = 0; i < 32; i++) v2_max[i] = (float)(i + 1) * 1.5f;
    r = set_value("k_max", "v_max", 32, v2_max, p_max);
    CHECK_EQ_INT("set_value k_max N=32", r, 0);

    // 18) get_value() para k_max y comprobacion
    memset(out_v1, 0, sizeof(out_v1));
    out_N = 0;
    memset(out_v2, 0, sizeof(out_v2));
    out_p.x = out_p.y = out_p.z = 0;
    r = get_value("k_max", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value k_max OK", r, 0);
    CHECK_EQ_STR("value k_max correcto", out_v1, "v_max");
    CHECK_EQ_INT("N k_max correcto", out_N, 32);
    if (!check_float_vec(out_v2, v2_max, 32)) {
        printf("[FAIL] V_value2 k_max no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 k_max coincide\n");
    }

    // 19) set_value() con V_value2 NULL -> -1
    r = set_value("k_null", "v", 1, NULL, p1);
    CHECK_EQ_INT("set_value V_value2 NULL", r, -1);

    // 20) set_value() con key demasiado larga -> -1
    char long_key[260];
    for (int i = 0; i < 256; i++) long_key[i] = 'x';
    long_key[256] = '\0'; // longitud 256 -> fuera de rango (max 255)
    r = set_value(long_key, "v", 1, v2_min, p1);
    CHECK_EQ_INT("set_value key demasiado larga", r, -1);

    // 21) set_value() con value1 demasiado largo -> -1
    char long_value1[260];
    for (int i = 0; i < 256; i++) long_value1[i] = 'y';
    long_value1[256] = '\0';
    r = set_value("k_longv", long_value1, 1, v2_min, p1);
    CHECK_EQ_INT("set_value value1 demasiado largo", r, -1);

    // 22) exist(NULL) -> -1
    r = exist(NULL);
    CHECK_EQ_INT("exist NULL", r, -1);

    // 23) get_value con punteros NULL -> -1
    r = get_value(NULL, out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value key NULL", r, -1);
    r = get_value("k_min", NULL, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value value1 NULL", r, -1);
    r = get_value("k_min", out_v1, NULL, out_v2, &out_p);
    CHECK_EQ_INT("get_value N_value2 NULL", r, -1);
    r = get_value("k_min", out_v1, &out_N, NULL, &out_p);
    CHECK_EQ_INT("get_value V_value2 NULL", r, -1);
    r = get_value("k_min", out_v1, &out_N, out_v2, NULL);
    CHECK_EQ_INT("get_value value3 NULL", r, -1);

    // 24) modify_value con N limites (1 y 32) -> OK
    float mv2_1[1] = {3.14f};
    r = modify_value("k_min", "v_min_mod", 1, mv2_1, p_min);
    CHECK_EQ_INT("modify_value k_min N=1", r, 0);
    float mv2_32[32];
    for (int i = 0; i < 32; i++) mv2_32[i] = (float)i;
    r = modify_value("k_max", "v_max_mod", 32, mv2_32, p_max);
    CHECK_EQ_INT("modify_value k_max N=32", r, 0);

    // 25) get_value tras modify para k_min y k_max
    r = get_value("k_min", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value k_min tras modify", r, 0);
    CHECK_EQ_STR("value k_min tras modify", out_v1, "v_min_mod");
    CHECK_EQ_INT("N k_min tras modify", out_N, 1);
    if (!check_float_vec(out_v2, mv2_1, 1)) {
        printf("[FAIL] V_value2 k_min tras modify no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 k_min tras modify coincide\n");
    }

    r = get_value("k_max", out_v1, &out_N, out_v2, &out_p);
    CHECK_EQ_INT("get_value k_max tras modify", r, 0);
    CHECK_EQ_STR("value k_max tras modify", out_v1, "v_max_mod");
    CHECK_EQ_INT("N k_max tras modify", out_N, 32);
    if (!check_float_vec(out_v2, mv2_32, 32)) {
        printf("[FAIL] V_value2 k_max tras modify no coincide\n");
        return 1;
    } else {
        printf("[ OK ] V_value2 k_max tras modify coincide\n");
    }

    // 26) limpieza final: borrar claves creadas y destroy
    CHECK_EQ_INT("delete_key k_min", delete_key("k_min"), 0);
    CHECK_EQ_INT("delete_key k_max", delete_key("k_max"), 0);
    CHECK_EQ_INT("delete_key k_null", delete_key("k_null"), -1);
    CHECK_EQ_INT("delete_key k_longv", delete_key("k_longv"), -1);
    CHECK_EQ_INT("delete_key k_max (de nuevo)", delete_key("k_max"), -1);

    r = destroy();
    CHECK_EQ_INT("destroy() final 2", r, 0);

    return 0;
}