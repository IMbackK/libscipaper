/*
 * Copyright (c) 2013 Yaroslav Stavnichiy <yarosla@gmail.com>
 *
 * This file is part of NXJSON.
 *
 * NXJSON is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * NXJSON is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NXJSON. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NXJSON_H
#define NXJSON_H

#ifdef  __cplusplus
extern "C" {
#endif

/**
* @defgroup JSONAPI Json Parsing API
* @{
*/

/**
 * @brief This enum is used in nx_json to discribe what type a given json entry has
 */
typedef enum nx_json_type {
  NX_JSON_NULL,    /**< this is null value */
  NX_JSON_OBJECT,  /**< this is an object; properties can be found in child nodes */
  NX_JSON_ARRAY,   /**< this is an array; items can be found in child nodes */
  NX_JSON_STRING,  /**< this is a string; value can be found in text_value field */
  NX_JSON_INTEGER, /**< this is an integer; value can be found in int_value field */
  NX_JSON_DOUBLE,  /**< this is a double; value can be found in dbl_value field */
  NX_JSON_BOOL     /**< this is a boolean; value can be found in int_value field */
} nx_json_type;

/**
 * @brief This struct discribes a parsed entry or file
 */
typedef struct nx_json {
  nx_json_type type;       /**< type of json node, see above */
  const char* key;         /**< key of the property; for object's children only */
  const char* text_value;  /**< text value of STRING node */
  long long int_value;     /**< the value of INTEGER or BOOL node */
  double dbl_value;        /**< the value of DOUBLE node */
  int length;              /**< number of children of OBJECT or ARRAY */
  struct nx_json* child;   /**< points to first child */
  struct nx_json* next;    /**< points to next child */
  struct nx_json* last_child;
} nx_json;

/**
 * @brief this typdef descibes a function that takes a Unicode codepoint and writes corresponding encoded value into buffer pointed by p.
 * It should store pointer to the end of encoded value into *endp. The function should return 1 on success and 0 on error. Number of bytes written must not exceed 6.
 */
typedef int (*nx_json_unicode_encoder)(unsigned int codepoint, char* p, char** endp);

extern nx_json_unicode_encoder nx_json_unicode_to_utf8;

/**
 * @brief This struct discribes a parsed entry or file
 * @param text a text string containing the json to be parsed
 * @param encoder a function of type nx_json_unicode_encoder
 * @return parsed top level json object as a nx_json struct, to be freed with nx_json_free() or NULL if there was a parse error
 */
const nx_json* nx_json_parse(char* text, nx_json_unicode_encoder encoder);

/**
 * @brief This is shortcut for nx_json_parse(text, nx_json_unicode_to_utf8) where nx_json_unicode_to_utf8 is unicode to UTF-8 encoder provided by NXJSON.
 * @param text a text string containing the json to be parsed
 * @return parsed top level json object as a nx_json struct, to be freed with nx_json_free() or NULL if there was a parse error
 */
const nx_json* nx_json_parse_utf8(char* text);

/**
 * @brief Frees a nx_json struct parsed by nx_json_parse()
 */
void nx_json_free(const nx_json* js);

/**
 * @brief Gets object's property or child by key.
 * This function never returns NULL or fails for any reason.
 * If a parse error occures or the requested key dosent exist a object of type NX_JSON_NULL is returned.
 * @param json parent json object
 * @param key key of the child or property you want
 * @return a nx_json struct discribeing the array element. Do not free this, its lifetime is equal to the top level nx_json object returned by nx_json_parse()
 */
const nx_json* nx_json_get(const nx_json* json, const char* key);

/**
 * @brief Gets an array element by index
 * This function never returns NULL or fails for any reason.
 * If a parse error occures or the requested key dosent exist a object of type NX_JSON_NULL is returned.
 * @param json The array
 * @param idx The index where you want to get the object out of the array
 * @return a nx_json struct discribeing the array element. Do not free this, its lifetime is equal to the top level nx_json object returned by nx_json_parse()
 */
const nx_json* nx_json_item(const nx_json* json, int idx);

/**@}*/

#ifdef  __cplusplus
}
#endif

#endif  /* NXJSON_H */
