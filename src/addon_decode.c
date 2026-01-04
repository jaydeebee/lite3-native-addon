#include <node_api.h>
#include <lite3-napi.h>
#include <lite3_context_api.h>

napi_status
lite3_napi_decode_value(napi_env env, lite3_ctx *ctx, size_t offset, enum lite3_type type, lite3_str *key, int index, napi_value *result) {
    if (!ctx) {
        napi_throw_error(env, NULL, "Invalid Lite3 context");
        return napi_generic_failure;
    }

    const char *key_ptr = key ? key->ptr : NULL;

    switch (type) {
        case LITE3_TYPE_OBJECT: {
            // If needed, adjust offset to point to the array object, not the parent:
            if (index >= 0 && lite3_ctx_arr_get_obj(ctx, offset, index, &offset)) {
                napi_throw_error(env, NULL, "Failed to get object from array");
                return napi_generic_failure;
            }

            // Create result object:
            napi_value dest_value;
            napi_status status = napi_create_object(env, &dest_value);
            if (status != napi_ok) {
                return status;
            }

            // Create an iterator over the lite3 object's properties:
            lite3_iter source_iter;
            LITE3_CALL(env, NULL, lite3_ctx_iter_create(ctx, offset, &source_iter), napi_generic_failure);

            // Walk through each property:
            size_t prop_offset;
            lite3_str prop_key;
            while (lite3_ctx_iter_next(ctx, &source_iter, &prop_key, &prop_offset) == LITE3_ITER_ITEM) {
                // Setup to decode the property value:
                enum lite3_type prop_type = lite3_ctx_get_type(ctx, offset, prop_key.ptr);
                size_t child_offset = prop_type == LITE3_TYPE_OBJECT || prop_type == LITE3_TYPE_ARRAY ? prop_offset : offset;
                lite3_str *child_prop_key = prop_type == LITE3_TYPE_OBJECT ? NULL : &prop_key;

                // Call the decoder recursively:
                napi_value prop_value;
                NAPI_CALL(env, NULL, lite3_napi_decode_value(env, ctx, child_offset, prop_type, child_prop_key, -1, &prop_value), napi_generic_failure);

                // Set the property on the destination object:
                napi_value key_str;
                NAPI_CALL(env, NULL, napi_create_string_utf8(env, prop_key.ptr, strlen(prop_key.ptr), &key_str), napi_generic_failure);
                NAPI_CALL(env, NULL, napi_set_property(env, dest_value, key_str, prop_value), napi_generic_failure);
            }
            *result = dest_value;
            break;
        }

        case LITE3_TYPE_ARRAY: {
            // Adjust offset to point to the array object, not the parent:
            if (index >= 0 && lite3_ctx_arr_get_arr(ctx, offset, index, &offset)) {
                napi_throw_error(env, NULL, "Failed to get object from array");
                return napi_generic_failure;
            }

            napi_value arr;
            napi_status status = napi_create_array(env, &arr);
            if (status != napi_ok) {
                return status;
            }

            lite3_iter arr_iter;
            LITE3_CALL(env, NULL, lite3_ctx_iter_create(ctx, offset, &arr_iter), napi_generic_failure);

            uint32_t i = 0;
            size_t elem_offset;
            for (
                int rc = lite3_ctx_iter_next(ctx, &arr_iter, NULL, &elem_offset);
                rc == LITE3_ITER_ITEM;
                rc = lite3_ctx_iter_next(ctx, &arr_iter, NULL, &elem_offset)
            ) {
                napi_value elem_value;
                enum lite3_type elem_type = lite3_ctx_arr_get_type(ctx, offset, i);
                NAPI_CALL(env, NULL, lite3_napi_decode_value(env, ctx, offset, elem_type, NULL, i, &elem_value), napi_generic_failure);
                NAPI_CALL(env, NULL, napi_set_element(env, arr, i, elem_value), napi_generic_failure);
                i++;
            }

            *result = arr;
            break;
        }

        case LITE3_TYPE_BOOL: {
            bool bool_value;
            int rc = index >= 0
                ? lite3_ctx_arr_get_bool(ctx, offset, index, &bool_value)
                : lite3_ctx_get_bool(ctx, offset, key_ptr, &bool_value);
            if (rc) {
                napi_throw_error(env, NULL, "Failed to get boolean value");
                return napi_generic_failure;
            }

            napi_value napi_boolean;
            NAPI_CALL(env, NULL, napi_get_boolean(env, bool_value, &napi_boolean), napi_generic_failure);
            *result = napi_boolean;
            break;
        }

        case LITE3_TYPE_F64: {
            double num_value;

            // Gather the number value:
            int rc = index >= 0
                ? lite3_ctx_arr_get_f64(ctx, offset, index, &num_value)
                : lite3_ctx_get_f64(ctx, offset, key_ptr, &num_value);
            if (rc) {
                napi_throw_error(env, NULL, "Failed to get double value");
                return napi_generic_failure;
            }

            napi_value napi_number;
            NAPI_CALL(env, NULL, napi_create_double(env, num_value, &napi_number), napi_generic_failure);
            *result = napi_number;
            break;
        }

        case LITE3_TYPE_I64: {
            int64_t int_value;

            // Gather the integer value:
            int rc = index >= 0
                ? lite3_ctx_arr_get_i64(ctx, offset, index, &int_value)
                : lite3_ctx_get_i64(ctx, offset, key_ptr, &int_value);
            if (rc) {
                napi_throw_error(env, NULL, "Failed to get integer value");
                return napi_generic_failure;
            }

            lite3_ctx_get_i64(ctx, offset, key_ptr, &int_value);
            napi_value napi_number;
            NAPI_CALL(env, NULL, napi_create_int64(env, int_value, &napi_number), napi_generic_failure);
            *result = napi_number;
            break;
        }

        case LITE3_TYPE_NULL: {
            NAPI_CALL(env, NULL, napi_get_null(env, result), napi_generic_failure);
            break;
        }

        case LITE3_TYPE_STRING: {
            lite3_str str_value;

            int rc =  index >= 0
                ? lite3_ctx_arr_get_str(ctx, offset, index, &str_value)
                : lite3_ctx_get_str(ctx, offset, key_ptr, &str_value);
            if (rc) {
                napi_throw_error(env, NULL, "Failed to get string value");
                return napi_generic_failure;
            }

            napi_value napi_string;
            NAPI_CALL(env, NULL, napi_create_string_utf8(env, str_value.ptr, str_value.len, &napi_string), napi_generic_failure);
            *result = napi_string;
            break;
        }

        case LITE3_TYPE_BYTES:
        case LITE3_TYPE_COUNT:
        case LITE3_TYPE_INVALID:
        default:
            // Unsupported type for top-level value
            napi_throw_error(env, NULL, "Unsupported value type in Lite3 buffer");
            return napi_generic_failure;
    }

    return napi_ok;
}

// Given a buffer, decode it into an object/array.
napi_value
decode(napi_env env, napi_callback_info info) {
    // Retrieve callback arguments into argv
    size_t argc = 1;
    napi_value argv[1];
    NAPI_CALL(env, NULL, napi_get_cb_info(env, info, &argc, argv, NULL, NULL), NULL);
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "Expected one argument");
        return NULL;
    }

    // Check type of argv[0]:
    bool is_buffer;
    if (napi_is_buffer(env, argv[0], &is_buffer) != napi_ok || !is_buffer) {
         napi_throw_type_error(env, NULL, "Argument must be a Buffer");
         return NULL;
    }

    // argv[0] is a Buffer to be decoded.
    void *buffer;
    size_t buffer_length;
    NAPI_CALL(env, NULL, napi_get_buffer_info(env, argv[0], &buffer, &buffer_length), NULL);

    // Create lite3 context from buffer:
    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_length);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type root_type = (enum lite3_type)*(ctx-> buf);

    // Decode the buffer into a napi_value:
    napi_value result;
    NAPI_CALL(
        env,
        ctx,
        lite3_napi_decode_value(
            env, // napi_env
            ctx, // lite3_ctx*
            0,   // offset (0 == root)
            LITE3_TYPE_OBJECT, // type: TODO: determine dynamically
            NULL, // key (NULL == root, or not an object)
            -1, // index (-1 == not an array index)
            &result // [out] result
        ),
        NULL);

#ifdef LITE3_DEBUG && LITE3_JSON
    lite3_ctx_json_print(ctx, 0); // For debugging
#endif

    lite3_ctx_destroy(ctx);

    return result;
}
