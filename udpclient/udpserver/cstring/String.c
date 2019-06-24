/* Created :
* Date : May 2016
* Filename: String.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "String.h"
string_t* string_init(void)
{
    string_t* str               =   NULL;
    char* under_lying_array     =   (char*)calloc(200, sizeof(char));

    if (under_lying_array != NULL)
    {
        str = (string_t*)calloc(1, sizeof(string_t));

        if (str != NULL)
        {
            str->char_array             =   under_lying_array;
            str->capacity               =   200;
            str->length                 =   0;
        }
        else
        {
            free(under_lying_array);
        }
    }

    return str;
}


string_t* string_cust_init(int initial_size)
{
    string_t* str               =   NULL;
    char* under_lying_array     =   NULL;

    if (initial_size > 0)
    {
        under_lying_array = (char*)calloc(initial_size, sizeof(char));

        if (under_lying_array != NULL)
        {
            str = (string_t*)calloc(1, sizeof(string_t));

            if (str != NULL)
            {
                str->char_array     =   under_lying_array;
                str->capacity       =   initial_size;
                str->length         =   0;
            }
            else
            {
                free(under_lying_array);
            }
        }
    }

    return str;
}
void string_destroy(string_t* str)
{
    if (str != NULL)
    {
        free(str->char_array);
        free(str);
    }
}

int string_equal(const string_t* str1, const string_t* str2)
{
    const char* str1_cptr;
    const char* str2_cptr;
    int return_val = 0;

    if (str1 != NULL && str2 != NULL)
    {
        if (str1->length == str2->length)
        {
            return_val = 1;

            for (str1_cptr = str1->char_array, str2_cptr = str2->char_array;
                 *str1_cptr != '\0' && *str2_cptr != '\0'; str1_cptr++, str2_cptr++)
            {
                if (*str1_cptr != *str2_cptr)
                    return_val = 0;
            }
        }
        else
        {
            return_val = 0;
        }
    }
    else if (str1 == NULL && str2 == NULL)
    {
        return_val = 1;
    }
    else
    {
        return_val = 0;
    }

    return return_val;
}

void string_copy_sptr(string_t* dest, const string_t* source) /* add dynamic memory allocation feature here */
{
    char* dest_cptr;
    const char* source_cptr;
    char* temp = NULL;

    if (dest != NULL && source != NULL)
    {
        if (source->length > 0)
        {
            if (dest->capacity < source->length + 1)
            {
                temp = (char*)realloc(dest->char_array, source->length + 51);

                if (temp != NULL)
                {
                    dest->char_array        =       temp;
                    dest->capacity          =       source->length + 51;
                }
            }

            if (dest->capacity >= source->length + 1)
            {
                dest->length = 0;
                for (source_cptr = source->char_array, dest_cptr = dest->char_array; *source_cptr != '\0';
                    source_cptr++, dest_cptr++)
                {
                    *dest_cptr = *source_cptr;
                    dest->length++;
                }
                *dest_cptr = *source_cptr; /* copy '\0' */
            }
        }
    }
}


void string_copy_cptr_sptr(string_t* dest, const char* source)
{
    int source_pos;
    char* temp = NULL;

    if (dest != NULL && source != NULL)
    {
        if (strlen(source) > 0)
        {
            if (dest->capacity < (int)(strlen(source)) + 1)
            {

                temp = (char*)realloc(dest->char_array, strlen(source) + 51);

                if (temp != NULL)
                {
                    dest->char_array        =       temp;
                    dest->capacity          =       strlen(source) + 51;
                }
            }

            if (dest->capacity >= (int)(strlen(source)) + 1)
            {
                dest->length = 0;
                for (source_pos = 0; source_pos < (int)(strlen(source)); source_pos++)
                {
                    dest->char_array[source_pos] = source[source_pos];
                    dest->length++;
                }

                dest->char_array[source_pos + 1] = '\0'; /* mark the end of the string with a null character */
            }
        }
    }
}
void string_concat(string_t* str1, const string_t* str2)
{
    int str1_pos, str2_pos;
    char* temp = NULL;

    if (str1 != NULL && str2 != NULL)
    {
        if (str2->length > 0)
        {
            if (str1->capacity < str1->length + str2->length + 2) /* + 2 for the '\0' characters */
            {
                temp = realloc(str1->char_array, str1->length + str2->length + 52);

                if (temp != NULL)
                {
                    str1->char_array        =       temp;
                    str1->capacity          =       str1->length + str2->length + 52;
                }
            }

            if (str1->capacity >= str1->length + str2->length + 2)
            {
                for (str1_pos = str1->length, str2_pos = 0; str2_pos < str2->length; str1_pos++, str2_pos++)
                {
                    str1->char_array[str1_pos] = str2->char_array[str2_pos];
                    str1->length++;
                }

                str1->char_array[str1_pos] = str2->char_array[str2_pos];
            }
        }
    }
}



void string_tolower(string_t* str)
{
    char* str_cptr;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if (*str_cptr >= 'A' && *str_cptr <= 'Z')
                    *str_cptr += 32;
            }
        }
    }
}

void string_toupper(string_t* str)
{
    char* str_cptr;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if (*str_cptr >= 'a' && *str_cptr <= 'z')
                    *str_cptr -= 32;
            }
        }
    }
}
void string_remove_nonalpha(string_t* str)
{
    int str_inner_pos, str_outer_pos;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_outer_pos = 0; str_outer_pos < str->length; str_outer_pos++)
            {
                if (!((str->char_array)[str_outer_pos] >= 'a' && (str->char_array)[str_outer_pos] <= 'z')
                    && !((str->char_array)[str_outer_pos] >= 'A' && (str->char_array)[str_outer_pos] <= 'Z'))
                {
                    for (str_inner_pos = str_outer_pos; str_inner_pos < str->length; str_inner_pos++)
                        (str->char_array)[str_inner_pos] = (str->char_array)[str_inner_pos + 1];
                    str->length--;
                    str_outer_pos--;
                }
            }
        }
    }

}

int string_isalpha(const string_t* str)
{
    const char* str_cptr;
    int return_val = 1;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if (!(*str_cptr >= 'a' && *str_cptr <= 'z') && !(*str_cptr >= 'A' && *str_cptr <= 'Z'))
                {
                    return_val = 0;
                    break;
                }
            }
        }
    }

    return return_val;
}


int string_isalphanum(const string_t* str)
{
    const char* str_cptr;
    int return_val = 1;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if (!(*str_cptr >= 'a' && *str_cptr <= 'z') && !(*str_cptr >= 'A' && *str_cptr <= 'Z')
                    && !(*str_cptr >= '0' && *str_cptr <= '9'))
                {
                        return_val = 0;
                        break;
                }
            }
        }
        else
        {
            return_val = 0;
        }
    }
    else
    {
        return_val = 0;
    }

    return return_val;
}

int string_isnonalpha(const string_t* str)
{
    const char* str_cptr;
    int return_val = 1;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if ((*str_cptr >= 'a' && *str_cptr <= 'z') || (*str_cptr >= 'A' && *str_cptr <= 'Z'))
                {
                    return_val = 0;
                    break;
                }
            }
        }
    }

    return return_val;
}

static double powi(double num,int n)//计算num的n次幂，其中n为整数
{
   double powint=1;
   int i;
   for(i=1;i<=n;i++) powint*=num;
   return powint;
}

int string_toint(const string_t* str)
{
    int highest_power;
    int return_val;
    const char* str_cptr;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            str_cptr = str->char_array;

            if (string_is_int(str))
            {
                return_val = 0;
                highest_power = strlen(str->char_array) - 1;

                while (*str_cptr != '\0')
                {
                    return_val += (*str_cptr - '0') * (int)powi(10, highest_power);
                    str_cptr++;
                    highest_power--;
                }
            }
            else
            {
                return_val = -3;
            }
        }
        else
        {
            return_val = -2;
        }
    }
    else
    {
        return_val = -1;
    }

    return return_val;
}

int string_equal_cstring(const string_t* str1, const char* str2)
{
    const char* str1_cptr;
    const char* str2_cptr;
    int return_val = 1;

    if (str1 != NULL && str2 != NULL)
    {
        if (str1->length == (int)(strlen(str2)))
        {
            for (str1_cptr = str1->char_array, str2_cptr = str2; *str1_cptr != '\0'; str1_cptr++, str2_cptr++)
            {
                if (*str1_cptr != *str2_cptr)
                {
                    return_val = 0;
                    break;
                }
            }
        }
        else
        {
            return_val = 0;
        }
    }
    else
    {
        return_val = 0;
    }

    return return_val;
}

void string_print(string_t* str, int add_new_line)
{
    if (str != NULL)
    {
        if (str->length > 0)
        {
            if (add_new_line)
                puts(str->char_array);
            else
                printf("%s", str->char_array);
        }
    }
}

int string_is_int(const string_t* str)
{
    const char* str_cptr;
    int return_val = 1;

    if (str != NULL)
    {
        if (str->length > 0)
        {
            for (str_cptr = str->char_array; *str_cptr != '\0'; str_cptr++)
            {
                if (!(*str_cptr >= '0' && *str_cptr <= '9') && *str_cptr != '-')
                {
                    return_val = 0;
                    break;
                }
            }

            for (str_cptr = str->char_array + 1; *str_cptr != '\0'; str_cptr++)
            {
                if (*str_cptr == '-')
                {
                    return_val = 0;
                    break;
                }
            }
        }
        else
        {
            return_val = 0;
        }
    }
    else
    {
        return_val = 0;
    }

    return return_val;
}

string_t* string_scan_s(void)
{
    string_t* stdin_data = string_init();
    string_scan2_s(stdin_data);

    return stdin_data;
}

void string_scan2_s(string_t* dest)
{
    char curr_char;
    char* buffer            =   calloc(200, sizeof(char));
    int sizeof_buffer       =   200;
    int len                 =   0;
    char* temp              =   NULL;

    curr_char = getc(stdin);
    if (buffer != NULL)
    {
        while (curr_char != '\n')
        {
            if (len == sizeof_buffer)
            {
                temp = (char*)realloc(buffer, sizeof_buffer + 50);
                if (temp != NULL)
                {
                    buffer              =       temp;
                    sizeof_buffer       +=      50;
                    strncat(buffer, &curr_char, 1);
                    len++;
                }
                else
                {
                    free(buffer);
                    buffer = NULL;
                    break;
                }
            }
            else
            {
                strncat(buffer, &curr_char, 1);
                len++;
            }

            curr_char = getc(stdin);
        }

        if (buffer != NULL)
        {
            string_copy_cptr_sptr(dest, buffer);
            free(buffer);
        }
    }
}

string_t* string_get_slice(const string_t* source, int beg_slice_indx, int end_slice_indx)
{
    string_t* source_slice          =       NULL;
    int bool_copy                   =       0;

    if (source != NULL)
    {
        if (source->length > 0)
        {
            if (beg_slice_indx >= 0 && beg_slice_indx < source->length)
            {
                if (end_slice_indx > 0 && end_slice_indx <= source->length)
                {
                    if (beg_slice_indx < end_slice_indx)
                    {

                        source_slice = string_cust_init((end_slice_indx - beg_slice_indx) + 1);
                        if (source_slice != NULL)
                        {
                            bool_copy = 1;
                            end_slice_indx--;
                        }
                    }
                }
                else if (end_slice_indx == -1)
                {
                    source_slice = string_cust_init((source->length - beg_slice_indx) + 1);
                    if (source_slice != NULL)
                    {
                        bool_copy           =       1;
                        end_slice_indx      =       source->length -1;
                    }
                }
            }
            else if (beg_slice_indx == -1 && end_slice_indx == -1)
            {
                source_slice = string_cust_init(source->length + 1);
                if (source_slice != NULL)
                {
                    bool_copy           =       1;
                    beg_slice_indx      =       0;
                    end_slice_indx      =       source->length - 1;
                }
            }

            if (bool_copy)
                string_copy_range(source_slice, source, beg_slice_indx,
                                  (end_slice_indx - beg_slice_indx) + 1);
        }
    }

    return source_slice;
}

void string_copy_range(string_t* dest, const string_t* source, int src_strt_idx, int num_to_copy)
{
    char* temp = NULL;
    int dest_pos, source_pos;

    if (dest != NULL && source != NULL)
    {
        if (source->length > 0)
        {
            if (src_strt_idx >= 0 && src_strt_idx < source->length - 1)
            {
                if (num_to_copy > 0)
                {
                    if (num_to_copy > (source->length - src_strt_idx))
                        num_to_copy = source->length - src_strt_idx;

                    if (num_to_copy <= dest->capacity)
                    {
                        dest->length = 0;
                        for (dest_pos = 0, source_pos = src_strt_idx;
                             dest_pos < num_to_copy; dest_pos++, source_pos++)
                        {
                            dest->char_array[dest_pos] = source->char_array[source_pos];
                            dest->length++;
                        }
                        dest->char_array[dest_pos] = '\0';
                    }
                    else
                    {
                        temp = (char*)realloc(dest->char_array, num_to_copy + 51);
                        if (temp != NULL)
                        {
                            dest->char_array        =       temp;
                            dest->capacity          =       num_to_copy + 51;
                            dest->length            =       0;
                            for (dest_pos = 0, source_pos = src_strt_idx;
                                 dest_pos < num_to_copy; dest_pos++, source_pos++)
                            {
                                dest->char_array[dest_pos] = source->char_array[source_pos];
                                dest->length++;
                            }
                            dest->char_array[dest_pos] = '\0';
                        }
                    }
                }
            }
        }
    }
}


void string_fgets(string_t* dest_str, int len, FILE* strm)
{
    char* temp = NULL;

    if (dest_str != NULL && strm != NULL)
    {
        if (len > 0)
        {
            if (len + 2 <= dest_str->capacity)
            {
                fgets(dest_str->char_array, len, strm);
                dest_str->length = len;
            }
            else
            {
                temp = (char*)realloc(dest_str->char_array, len + 52);
                if (temp != NULL)
                {
                    dest_str->capacity          =       len + 52;
                    dest_str->char_array        =       temp;

                    fgets(dest_str->char_array, len, strm);
                    dest_str->length = len;
                }
            }
        }
    }
}


int string_in(string_t* str_to_search, char* str_to_find)
{
    int ret_val = 0;

    if (str_to_search != NULL && str_to_find != NULL)
    {
        if (str_to_search->length > 0 && strlen(str_to_find) > 0)
        {
            if (string_find_cstr(str_to_search, str_to_find, 0) >= 0)
                ret_val = 1;
        }
    }

    return ret_val;
}


int string_in2(string_t* str_to_search, string_t* str_to_find)
{
    return string_in(str_to_search, str_to_find->char_array);
}


int string_find_cstr(string_t* str_to_search, char* str_to_find, int search_strt_pos)
{
    int ret_val = -1, str_to_find_pos, str_to_search_pos;

    if (str_to_search != NULL && str_to_find != NULL)
    {
        if (search_strt_pos >= 0 && search_strt_pos < str_to_search->length)
        {
            if (strlen(str_to_find) > 0 && str_to_search->length > 0)
            {
                str_to_find_pos         =       0;
                str_to_search_pos       =       search_strt_pos;

                while (str_to_search_pos < str_to_search->length)
                {
                    if (str_to_search->char_array[str_to_search_pos] == str_to_find[str_to_find_pos])
                    {
                        ret_val = str_to_search_pos;
                        str_to_find_pos++;
                        str_to_search_pos++;

                        while (str_to_find_pos < (int)strlen(str_to_find) && str_to_search_pos < str_to_search->length)
                        {
                            if (str_to_search->char_array[str_to_search_pos] != str_to_find[str_to_find_pos])
                            {
                                ret_val             =       -1;
                                str_to_find_pos     =       0;
                                break;
                            }

                            str_to_find_pos++;
                            str_to_search_pos++;
                        }

                        if (str_to_find_pos > 0)
                            break;
                    }

                    str_to_search_pos++;
                }
            }
        }
    }

    return ret_val;
}


int string_find(string_t* str_to_search, string_t* str_to_find, int search_strt_pos)
{
    return string_find_cstr(str_to_search, str_to_find->char_array, search_strt_pos);
}


void string_set_range(string_t* dest, char* replacement_str, int start_pos, int end_pos)
{
    int dest_pos, replacement_str_pos, count_to_write;

    if (dest != NULL && replacement_str != NULL)
    {
        if (start_pos >= 0 && start_pos < dest->length && end_pos >= 0 && end_pos < dest->length)
        {
            if (start_pos < end_pos)
            {
                if ((int)strlen(replacement_str) > (end_pos - start_pos) + 1)
                    count_to_write = (end_pos - start_pos) + 1;
                else
                    count_to_write = (int)strlen(replacement_str);

                for (dest_pos = start_pos, replacement_str_pos = 0; replacement_str_pos < count_to_write;
                    dest_pos++, replacement_str_pos++)
                        dest->char_array[dest_pos] = replacement_str[replacement_str_pos];
            }
            else if (start_pos == end_pos)
            {
                dest->char_array[start_pos] = replacement_str[0];
            }
        }
    }
}
void string_set_range2(string_t* dest, string_t* replacement_str, int start_pos, int end_pos)
{
    string_set_range(dest, replacement_str->char_array, start_pos, end_pos);
}


int string_replace(string_t* dest, char* str_to_replace, char* replacement_text)
{
    static int search_strt_pos              =       0;
    int bool_replaced                       =       0;
    int size_diff                           =       0; 
    char* temp                              =       NULL;
    static string_t* old_dest               =       NULL;
    static char* old_str_to_replace         =       NULL;
    static char* old_replacement_text       =       NULL;
    int dest_pos, found_at_pos, replacement_end_pos;

    if (old_dest == NULL && old_str_to_replace == NULL && old_replacement_text == NULL)
    {
        old_dest                    =       dest;
        old_replacement_text        =       replacement_text;
        old_str_to_replace          =       str_to_replace;
    }

    if (dest != NULL && str_to_replace != NULL && replacement_text != NULL)
    {
        if (dest->length > 0 && strlen(str_to_replace) > 0 && strlen(replacement_text) > 0)
        {
            if (dest != old_dest || str_to_replace != old_str_to_replace || replacement_text != old_replacement_text)
            {
                search_strt_pos             =       0;
                old_dest                    =       dest;
                old_replacement_text        =       replacement_text;
                old_str_to_replace          =       str_to_replace;
            }

            size_diff           =       (int)(fabs((double)((int)strlen(str_to_replace) - (int)strlen(replacement_text))));
            found_at_pos        =       string_find_cstr(dest, str_to_replace, search_strt_pos);
            search_strt_pos     =       found_at_pos + 1;

            if (found_at_pos > -1)

            {
                if (strlen(replacement_text) > strlen(str_to_replace))
                {
                    if (dest->length + size_diff > dest->capacity)
                    {
                        temp = (char*)realloc(dest->char_array, dest->capacity + size_diff + 50);
                    }

                    if (temp != NULL || dest->length + size_diff <= dest->capacity)
                    {
                        if (temp != NULL)
                            dest->capacity = dest->capacity + size_diff + 50;


                        for (dest_pos = dest->length - 1; dest_pos > found_at_pos + (int)strlen(str_to_replace) - 1; dest_pos--)
                            dest->char_array[dest_pos + size_diff] = dest->char_array[dest_pos];

                        dest->length            +=      size_diff;
                        replacement_end_pos     =       found_at_pos + strlen(replacement_text) - 1;
                        string_set_range(dest, replacement_text, found_at_pos, replacement_end_pos);
                        bool_replaced           =       1;
                    }
                }
                else if (strlen(replacement_text) < strlen(str_to_replace))
                {

                    for (dest_pos = found_at_pos + strlen(replacement_text); dest_pos < dest->length - 1; dest_pos++)
                        dest->char_array[dest_pos] = dest->char_array[dest_pos + size_diff];

                    dest->length            -=      size_diff;
                    replacement_end_pos     =       found_at_pos + strlen(replacement_text) - 1;
                    string_set_range(dest, replacement_text, found_at_pos, replacement_end_pos);
                    bool_replaced           =       1;
                }
                else
                {
                    replacement_end_pos     =       found_at_pos + strlen(replacement_text) - 1;
                    string_set_range(dest, replacement_text, found_at_pos, replacement_end_pos);
                    bool_replaced           =       1;
                }
            }
        }
    }

    return bool_replaced;
}

int string_replace2(string_t* dest, string_t* str_to_replace, string_t* replacement_text)
{
    return string_replace(dest, str_to_replace->char_array, replacement_text->char_array);
}
