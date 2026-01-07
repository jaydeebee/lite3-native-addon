/**
 * Lite3 Proxy Support Functions
 *
 * N-API functions for lazy/proxy-based access to Lite3 buffers.
 * These enable accessing individual properties without decoding the entire buffer.
 */

#include <node_api.h>
#include <lite3-napi.h>
#include <lite3_context_api.h>
#include <string.h>

// Helper: extract buffer, offset, and key from arguments
static napi_status
extract_args_obj(napi_env env, napi_callback_info info,
                 void **buffer, size_t *buffer_len, int64_t *offset, char *key, size_t key_size) {
    size_t argc = 3;
    napi_value argv[3];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return status;

    if (argc < 3) {
        napi_throw_type_error(env, NULL, "Expected 3 arguments: buffer, offset, key");
        return napi_invalid_arg;
    }

    bool is_buffer;
    status = napi_is_buffer(env, argv[0], &is_buffer);
    if (status != napi_ok || !is_buffer) {
        napi_throw_type_error(env, NULL, "First argument must be a Buffer");
        return napi_invalid_arg;
    }

    status = napi_get_buffer_info(env, argv[0], buffer, buffer_len);
    if (status != napi_ok) return status;

    status = napi_get_value_int64(env, argv[1], offset);
    if (status != napi_ok) return status;

    status = napi_get_value_string_utf8(env, argv[2], key, key_size, NULL);
    if (status != napi_ok) return status;

    return napi_ok;
}

// Helper: extract buffer, offset, and index from arguments (for arrays)
static napi_status
extract_args_arr(napi_env env, napi_callback_info info,
                 void **buffer, size_t *buffer_len, int64_t *offset, uint32_t *index) {
    size_t argc = 3;
    napi_value argv[3];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return status;

    if (argc < 3) {
        napi_throw_type_error(env, NULL, "Expected 3 arguments: buffer, offset, index");
        return napi_invalid_arg;
    }

    bool is_buffer;
    status = napi_is_buffer(env, argv[0], &is_buffer);
    if (status != napi_ok || !is_buffer) {
        napi_throw_type_error(env, NULL, "First argument must be a Buffer");
        return napi_invalid_arg;
    }

    status = napi_get_buffer_info(env, argv[0], buffer, buffer_len);
    if (status != napi_ok) return status;

    status = napi_get_value_int64(env, argv[1], offset);
    if (status != napi_ok) return status;

    status = napi_get_value_uint32(env, argv[2], index);
    if (status != napi_ok) return status;

    return napi_ok;
}

// Helper: extract just buffer and offset (for getKeys, getLength)
static napi_status
extract_args_buf_ofs(napi_env env, napi_callback_info info,
                     void **buffer, size_t *buffer_len, int64_t *offset) {
    size_t argc = 2;
    napi_value argv[2];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return status;

    if (argc < 2) {
        napi_throw_type_error(env, NULL, "Expected 2 arguments: buffer, offset");
        return napi_invalid_arg;
    }

    bool is_buffer;
    status = napi_is_buffer(env, argv[0], &is_buffer);
    if (status != napi_ok || !is_buffer) {
        napi_throw_type_error(env, NULL, "First argument must be a Buffer");
        return napi_invalid_arg;
    }

    status = napi_get_buffer_info(env, argv[0], buffer, buffer_len);
    if (status != napi_ok) return status;

    status = napi_get_value_int64(env, argv[1], offset);
    if (status != napi_ok) return status;

    return napi_ok;
}

// Helper: convert lite3_type to JS string
static napi_status
type_to_string(napi_env env, enum lite3_type type, napi_value *result) {
    const char *type_str;
    switch (type) {
        case LITE3_TYPE_OBJECT:  type_str = "object"; break;
        case LITE3_TYPE_ARRAY:   type_str = "array"; break;
        case LITE3_TYPE_STRING:  type_str = "string"; break;
        case LITE3_TYPE_I64:     type_str = "number"; break;
        case LITE3_TYPE_F64:     type_str = "number"; break;
        case LITE3_TYPE_BOOL:    type_str = "boolean"; break;
        case LITE3_TYPE_NULL:    type_str = "null"; break;
        case LITE3_TYPE_BYTES:   type_str = "bytes"; break;
        default:                 type_str = "undefined"; break;
    }
    return napi_create_string_utf8(env, type_str, NAPI_AUTO_LENGTH, result);
}

/**
 * getType(buffer, offset, key) -> string
 * Returns the type of a property as a string: "object", "array", "string", "number", "boolean", "null"
 */
napi_value proxy_get_type(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    char key[256];

    if (extract_args_obj(env, info, &buffer, &buffer_len, &offset, key, sizeof(key)) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_get_type(ctx, (size_t)offset, key);
    lite3_ctx_destroy(ctx);

    napi_value result;
    if (type_to_string(env, type, &result) != napi_ok) {
        return NULL;
    }
    return result;
}

/**
 * getArrayType(buffer, offset, index) -> string
 * Returns the type of an array element as a string
 */
napi_value proxy_get_array_type(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    uint32_t index;

    if (extract_args_arr(env, info, &buffer, &buffer_len, &offset, &index) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_arr_get_type(ctx, (size_t)offset, index);
    lite3_ctx_destroy(ctx);

    napi_value result;
    if (type_to_string(env, type, &result) != napi_ok) {
        return NULL;
    }
    return result;
}

/**
 * getValue(buffer, offset, key) -> any
 * Decodes and returns a single primitive value (string, number, boolean, null)
 * For objects/arrays, use getChildOffset instead
 */
napi_value proxy_get_value(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    char key[256];

    if (extract_args_obj(env, info, &buffer, &buffer_len, &offset, key, sizeof(key)) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_get_type(ctx, (size_t)offset, key);
    napi_value result;

    switch (type) {
        case LITE3_TYPE_STRING: {
            lite3_str str;
            if (lite3_ctx_get_str(ctx, (size_t)offset, key, &str) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get string value");
                return NULL;
            }
            napi_create_string_utf8(env, str.ptr, str.len, &result);
            break;
        }
        case LITE3_TYPE_I64: {
            int64_t val;
            if (lite3_ctx_get_i64(ctx, (size_t)offset, key, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get integer value");
                return NULL;
            }
            napi_create_int64(env, val, &result);
            break;
        }
        case LITE3_TYPE_F64: {
            double val;
            if (lite3_ctx_get_f64(ctx, (size_t)offset, key, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get double value");
                return NULL;
            }
            napi_create_double(env, val, &result);
            break;
        }
        case LITE3_TYPE_BOOL: {
            bool val;
            if (lite3_ctx_get_bool(ctx, (size_t)offset, key, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get boolean value");
                return NULL;
            }
            napi_get_boolean(env, val, &result);
            break;
        }
        case LITE3_TYPE_NULL: {
            napi_get_null(env, &result);
            break;
        }
        case LITE3_TYPE_OBJECT:
        case LITE3_TYPE_ARRAY: {
            // For nested structures, return the offset as a number
            // Caller should use getChildOffset for these
            size_t child_offset;
            int rc = (type == LITE3_TYPE_OBJECT)
                ? lite3_ctx_get_obj(ctx, (size_t)offset, key, &child_offset)
                : lite3_ctx_get_arr(ctx, (size_t)offset, key, &child_offset);
            if (rc != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get child offset");
                return NULL;
            }
            napi_create_int64(env, (int64_t)child_offset, &result);
            break;
        }
        default: {
            napi_get_undefined(env, &result);
            break;
        }
    }

    lite3_ctx_destroy(ctx);
    return result;
}

/**
 * getArrayElement(buffer, offset, index) -> any
 * Decodes and returns a single array element
 */
napi_value proxy_get_array_element(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    uint32_t index;

    if (extract_args_arr(env, info, &buffer, &buffer_len, &offset, &index) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_arr_get_type(ctx, (size_t)offset, index);
    napi_value result;

    switch (type) {
        case LITE3_TYPE_STRING: {
            lite3_str str;
            if (lite3_ctx_arr_get_str(ctx, (size_t)offset, index, &str) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get string value");
                return NULL;
            }
            napi_create_string_utf8(env, str.ptr, str.len, &result);
            break;
        }
        case LITE3_TYPE_I64: {
            int64_t val;
            if (lite3_ctx_arr_get_i64(ctx, (size_t)offset, index, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get integer value");
                return NULL;
            }
            napi_create_int64(env, val, &result);
            break;
        }
        case LITE3_TYPE_F64: {
            double val;
            if (lite3_ctx_arr_get_f64(ctx, (size_t)offset, index, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get double value");
                return NULL;
            }
            napi_create_double(env, val, &result);
            break;
        }
        case LITE3_TYPE_BOOL: {
            bool val;
            if (lite3_ctx_arr_get_bool(ctx, (size_t)offset, index, &val) != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get boolean value");
                return NULL;
            }
            napi_get_boolean(env, val, &result);
            break;
        }
        case LITE3_TYPE_NULL: {
            napi_get_null(env, &result);
            break;
        }
        case LITE3_TYPE_OBJECT:
        case LITE3_TYPE_ARRAY: {
            size_t child_offset;
            int rc = (type == LITE3_TYPE_OBJECT)
                ? lite3_ctx_arr_get_obj(ctx, (size_t)offset, index, &child_offset)
                : lite3_ctx_arr_get_arr(ctx, (size_t)offset, index, &child_offset);
            if (rc != 0) {
                lite3_ctx_destroy(ctx);
                napi_throw_error(env, NULL, "Failed to get child offset");
                return NULL;
            }
            napi_create_int64(env, (int64_t)child_offset, &result);
            break;
        }
        default: {
            napi_get_undefined(env, &result);
            break;
        }
    }

    lite3_ctx_destroy(ctx);
    return result;
}

/**
 * getChildOffset(buffer, offset, key) -> number
 * Returns the offset of a nested object or array
 */
napi_value proxy_get_child_offset(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    char key[256];

    if (extract_args_obj(env, info, &buffer, &buffer_len, &offset, key, sizeof(key)) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_get_type(ctx, (size_t)offset, key);
    size_t child_offset;
    int rc;

    if (type == LITE3_TYPE_OBJECT) {
        rc = lite3_ctx_get_obj(ctx, (size_t)offset, key, &child_offset);
    } else if (type == LITE3_TYPE_ARRAY) {
        rc = lite3_ctx_get_arr(ctx, (size_t)offset, key, &child_offset);
    } else {
        lite3_ctx_destroy(ctx);
        napi_throw_error(env, NULL, "Property is not an object or array");
        return NULL;
    }

    lite3_ctx_destroy(ctx);

    if (rc != 0) {
        napi_throw_error(env, NULL, "Failed to get child offset");
        return NULL;
    }

    napi_value result;
    napi_create_int64(env, (int64_t)child_offset, &result);
    return result;
}

/**
 * getArrayChildOffset(buffer, offset, index) -> number
 * Returns the offset of a nested object or array within an array
 */
napi_value proxy_get_array_child_offset(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    uint32_t index;

    if (extract_args_arr(env, info, &buffer, &buffer_len, &offset, &index) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_arr_get_type(ctx, (size_t)offset, index);
    size_t child_offset;
    int rc;

    if (type == LITE3_TYPE_OBJECT) {
        rc = lite3_ctx_arr_get_obj(ctx, (size_t)offset, index, &child_offset);
    } else if (type == LITE3_TYPE_ARRAY) {
        rc = lite3_ctx_arr_get_arr(ctx, (size_t)offset, index, &child_offset);
    } else {
        lite3_ctx_destroy(ctx);
        napi_throw_error(env, NULL, "Element is not an object or array");
        return NULL;
    }

    lite3_ctx_destroy(ctx);

    if (rc != 0) {
        napi_throw_error(env, NULL, "Failed to get child offset");
        return NULL;
    }

    napi_value result;
    napi_create_int64(env, (int64_t)child_offset, &result);
    return result;
}

/**
 * getKeys(buffer, offset) -> string[]
 * Returns an array of keys for the object at the given offset
 */
napi_value proxy_get_keys(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;

    if (extract_args_buf_ofs(env, info, &buffer, &buffer_len, &offset) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    // Create result array
    napi_value result;
    NAPI_CALL(env, ctx, napi_create_array(env, &result), NULL);

    // Iterate and collect keys
    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, (size_t)offset, &iter) != 0) {
        lite3_ctx_destroy(ctx);
        napi_throw_error(env, NULL, "Failed to create iterator");
        return NULL;
    }

    uint32_t i = 0;
    lite3_str key;
    size_t val_ofs;
    while (lite3_ctx_iter_next(ctx, &iter, &key, &val_ofs) == LITE3_ITER_ITEM) {
        napi_value key_str;
        // Use strlen() as lite3_str.len may include extra data beyond the null terminator
        NAPI_CALL(env, ctx, napi_create_string_utf8(env, key.ptr, strlen(key.ptr), &key_str), NULL);
        NAPI_CALL(env, ctx, napi_set_element(env, result, i, key_str), NULL);
        i++;
    }

    lite3_ctx_destroy(ctx);
    return result;
}

/**
 * getLength(buffer, offset) -> number
 * Returns the length of an array or object at the given offset
 */
napi_value proxy_get_length(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;

    if (extract_args_buf_ofs(env, info, &buffer, &buffer_len, &offset) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    uint32_t count;
    if (lite3_ctx_count(ctx, (size_t)offset, &count) < 0) {
        lite3_ctx_destroy(ctx);
        napi_throw_error(env, NULL, "Failed to get element count");
        return NULL;
    }

    lite3_ctx_destroy(ctx);

    napi_value result;
    napi_create_uint32(env, count, &result);
    return result;
}

/**
 * hasKey(buffer, offset, key) -> boolean
 * Returns true if the object has the given key
 */
napi_value proxy_has_key(napi_env env, napi_callback_info info) {
    void *buffer;
    size_t buffer_len;
    int64_t offset;
    char key[256];

    if (extract_args_obj(env, info, &buffer, &buffer_len, &offset, key, sizeof(key)) != napi_ok) {
        return NULL;
    }

    lite3_ctx *ctx = lite3_ctx_create_from_buf(buffer, buffer_len);
    if (!ctx) {
        napi_throw_error(env, NULL, "Failed to create Lite3 context");
        return NULL;
    }

    enum lite3_type type = lite3_ctx_get_type(ctx, (size_t)offset, key);
    lite3_ctx_destroy(ctx);

    bool exists = (type != LITE3_TYPE_INVALID);

    napi_value result;
    napi_get_boolean(env, exists, &result);
    return result;
}

/**
 * getRootType(buffer) -> string
 * Returns the type of the root element
 */
napi_value proxy_get_root_type(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];

    if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok) {
        return NULL;
    }

    if (argc < 1) {
        napi_throw_type_error(env, NULL, "Expected 1 argument: buffer");
        return NULL;
    }

    void *buffer;
    size_t buffer_len;
    bool is_buffer;

    if (napi_is_buffer(env, argv[0], &is_buffer) != napi_ok || !is_buffer) {
        napi_throw_type_error(env, NULL, "Argument must be a Buffer");
        return NULL;
    }

    if (napi_get_buffer_info(env, argv[0], &buffer, &buffer_len) != napi_ok) {
        return NULL;
    }

    if (buffer_len == 0) {
        napi_throw_error(env, NULL, "Buffer is empty");
        return NULL;
    }

    // Root type is stored in first byte
    enum lite3_type type = (enum lite3_type)(*(uint8_t *)buffer);

    napi_value result;
    if (type_to_string(env, type, &result) != napi_ok) {
        return NULL;
    }
    return result;
}