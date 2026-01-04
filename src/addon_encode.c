#include <node_api.h>
#include <lite3-napi.h>
#include <lite3_context_api.h>
#include <stdlib.h>

static napi_status encode_enumerable(napi_env, napi_value, bool, lite3_ctx*, size_t);
static napi_status encode_element(napi_env, char*, napi_value, bool, lite3_ctx*, size_t);

static napi_status
encode_enumerable(napi_env env, napi_value value, bool is_array, lite3_ctx *ctx, size_t offset) {
    napi_status status;

    // Get property names and walk through each value
    napi_value prop_names;
    status = napi_get_property_names(env, value, &prop_names);
    if (status != napi_ok) return status;

    uint32_t prop_name_count;
    status = napi_get_array_length(env, prop_names, &prop_name_count);
    if (status != napi_ok) return status;

    for (uint32_t i = 0; i < prop_name_count; i++) {
        napi_value key;
        status = napi_get_element(env, prop_names, i, &key);
        if (status != napi_ok) return status;

        size_t key_len;
        status = napi_get_value_string_utf8(env, key, NULL, 0, &key_len);
        if (status != napi_ok) return status;
        char *key_str = malloc(key_len + 1);
        if (!key_str) {
            napi_throw_error(env, NULL, "Memory allocation failure");
            return napi_generic_failure;
        }
        status = napi_get_value_string_utf8(env, key, key_str, key_len + 1, &key_len);
        if (status != napi_ok) {
            free(key_str);
            return status;
        }
        napi_value property_value;
        status = napi_get_property(env, value, key, &property_value);
        if (status != napi_ok) {
            free(key_str);
            return status;
        }
        status = encode_element(env, key_str, property_value, is_array, ctx, offset);
        if (status != napi_ok) {
            free(key_str);
            return status;
        }
        // We don't free key_str here because lite3_ctx_set_* takes ownership of the string
    }

    return napi_ok;
}

static napi_status
encode_element(napi_env env, char *key_name, napi_value value, bool parent_is_array, lite3_ctx *ctx, size_t offset) {
    // Walk through each element in the array and encode it into ctx based on type.
    napi_valuetype type;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) return status;

    switch (type) {
        case napi_string: {
            size_t len;
            status = napi_get_value_string_utf8(env, value, NULL, 0, &len);
            if (status != napi_ok) return status;
            char *str = malloc(len + 1);
            if (!str) {
                napi_throw_error(env, NULL, "Memory allocation failure");
                return napi_generic_failure;
            }
            status = napi_get_value_string_utf8(env, value, str, len + 1, &len);
            if (status != napi_ok) {
                // free(str);
                return status;
            }
#ifdef LITE3_DEBUG
            printf("Encoding string key='%s' value='%s'\n", key_name, str);
#endif // LITE3_DEBUG
            if (parent_is_array) {
                int lite3_arr_append_str_result = lite3_ctx_arr_append_str(ctx, offset, str);
                if (lite3_arr_append_str_result != 0) {
                    free(str);
                    return napi_generic_failure;
                }
            } else {
                int lite3_set_str_result = lite3_ctx_set_str(ctx, offset, key_name, str);
                if (lite3_set_str_result != 0) {
                    free(str);
                    return napi_generic_failure;
                }
            }
            // We do not free str here, as lite3_ctx_set_str takes ownership of it
            return napi_ok;
        }

        // TODO: What is a bigint in lite3?
        // case napi_bigint: {
        // }

        case napi_number: {
            double num;
            status = napi_get_value_double(env, value, &num);
            if (status != napi_ok) return status;
#ifdef LITE3_DEBUG
            printf("Encoding number key='%s' value=%f\n", key_name, num);
#endif // LITE3_DEBUG
            if (parent_is_array) {
                LITE3_CALL(env, NULL, lite3_ctx_arr_append_f64(ctx, offset, num), napi_generic_failure);
            } else {
                LITE3_CALL(env, NULL, lite3_ctx_set_f64(ctx, offset, key_name, num), napi_generic_failure);
            }
            return napi_ok;
        }

        case napi_boolean: {
            bool b;
            status = napi_get_value_bool(env, value, &b);
            if (status != napi_ok) return status;
#ifdef LITE3_DEBUG
            printf("Encoding boolean key='%s' value=%s\n", key_name, b ?
                     "true" : "false");
#endif // LITE3_DEBUG
            if (parent_is_array) {
                LITE3_CALL(env, NULL, lite3_ctx_arr_append_bool(ctx, offset, b), napi_generic_failure);
            } else {
                LITE3_CALL(env, NULL, lite3_ctx_set_bool(ctx, offset, key_name, b), napi_generic_failure);
            }
            return napi_ok;
        }

        case napi_null: {
#ifdef LITE3_DEBUG
            printf("Encoding null key='%s'\n", key_name);
#endif // LITE3_DEBUG
            if (parent_is_array) {
                LITE3_CALL(env, NULL, lite3_ctx_arr_append_null(ctx, offset), napi_generic_failure);
            } else {
                LITE3_CALL(env, NULL, lite3_ctx_set_null(ctx, offset, key_name), napi_generic_failure);
            }
            return napi_ok;
        }

        // handles arrays and objects:
        case napi_object: {
            bool is_array;
            status = napi_is_array(env, value, &is_array);
            if (status != napi_ok) return status;

            size_t new_offset;
            if (parent_is_array) {
                if (is_array) {
                    LITE3_CALL(env, NULL, lite3_ctx_arr_append_arr(ctx, offset, &new_offset), napi_generic_failure);
                } else {
                    LITE3_CALL(env, NULL, lite3_ctx_arr_append_obj(ctx, offset, &new_offset), napi_generic_failure);
                }
            } else {
                if (is_array) {
                    LITE3_CALL(env, NULL, lite3_ctx_set_arr(ctx, offset, key_name, &new_offset), napi_generic_failure);
                } else {
                    LITE3_CALL(env, NULL, lite3_ctx_set_obj(ctx, offset, key_name, &new_offset), napi_generic_failure);
                }
            }

#ifdef LITE3_DEBUG
            printf("Encoding %s key='%s' at offset=%zu\n", is_array ? "array" : "object", key_name, new_offset);
#endif // LITE3_DEBUG

            // Pass new_offset as the base for the next level down.
            // We then return status directly, no need to check it:
            return encode_enumerable(env, value, is_array, ctx, new_offset);
        }

        default: {
            // Something unsupported:
            //  - function
            //  - undefined
            //  - symbol
            // We simply skip them from the output.
#ifdef LITE3_DEBUG
            printf("Skipping unsupported type for key='%s'\n", key_name);
#endif // LITE3_DEBUG
            return napi_ok;
        }
    }
}

// Encode the argument into a Buffer and return to caller
napi_value
encode(napi_env env, napi_callback_info info) {
    // Check type of `info`, must be object or array:
    size_t argc = 1;
    napi_value argv[1];
    NAPI_CALL(env, NULL, napi_get_cb_info(env, info, &argc, argv, NULL, NULL), NULL);
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "Expected one argument");
        return NULL;
    }

    // Check type of argv[0]:
    napi_valuetype type;
    NAPI_CALL(env, NULL, napi_typeof(env, argv[0], &type), NULL);
    if (type != napi_object) {
        napi_throw_type_error(env, NULL, "Argument must be an array or object");
        return NULL;
    }

    // Create a Lite3 context to receive our encoded data:
    lite3_ctx *ctx = lite3_ctx_create();
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    bool is_array;
    NAPI_CALL(env, ctx, napi_is_array(env, argv[0], &is_array), NULL);

    // Prime the Lite3 context with the appropriate type:
    if (is_array) LITE3_CALL(env, ctx, lite3_ctx_init_arr(ctx), NULL);
    else LITE3_CALL(env, ctx, lite3_ctx_init_obj(ctx), NULL);

    // Fill that context with element data:
    NAPI_CALL(env, ctx, encode_enumerable(env, argv[0], is_array, ctx, 0), NULL);

#ifdef LITE3_DEBUG && LITE3_JSON
    lite3_ctx_json_print(ctx, 0); // For debugging
#endif

    // Convert to a Buffer:
    // TODO: Can we not copy? Can we make this an external or something?
    napi_value result;
    NAPI_CALL(env, ctx, napi_create_buffer_copy(env, ctx->buflen, ctx->buf, NULL, &result), NULL);
    lite3_ctx_destroy(ctx);

    return result;
}

