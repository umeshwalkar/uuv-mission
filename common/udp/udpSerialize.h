/*
 * udpSerialize.h
 *
 *  Created on: 22 gen 2018
 *      Author: aitronik
 */

#ifndef __UDP_UDPSERIALIZE_H__
#define __UDP_UDPSERIALIZE_H__

#include <stdint.h>
#include <string.h>
// #include "../baseType.h"
#include <vector>

/** Serialize data for udp comunication */
template <typename T>
void pack(std::vector<uint8_t> *buff, T *data, bool swap)
{

    uint8_t *src = reinterpret_cast<uint8_t *>(data);
    if (swap && (sizeof(T) > 1))
    {
        for (uint16_t i = 0; i < sizeof(T); i++)
        {
            buff->insert(buff->end(), src[sizeof(T) - 1 - i]);
        }
    }
    else
    {
        buff->insert(buff->end(), src, src + sizeof(T));
    }
};

/** Deserialize data for udp comunication */
template <typename T>
void unpack(uint8_t *buff, int index, T *data, bool swap)
{
    if (swap && (sizeof(T) > 1))
    {
        uint8_t swapped[sizeof(T)];
        for (uint16_t i = 0; i < sizeof(T); i++)
        {
            swapped[i] = buff[index + sizeof(T) - 1 - i];
        }
        (*data) = (*reinterpret_cast<T *>(swapped));
    }
    else
    {
        std::copy(reinterpret_cast<T *>(&buff[index]), reinterpret_cast<T *>(&buff[index + sizeof(T)]), data);
    }
};

/* strtok_fixed - fixed variation of strtok,
 * The function returns 0 if the string is empty instead of null */
static char *strtok_fixed(char *str, char const *delims)
{
    static char *src = NULL;
    char *p, *ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL || *src == '\0') // Fix 1
        return NULL;

    ret = src; // Fix 2
    if ((p = strpbrk(src, delims)) != NULL)
    {
        *p = 0;
        // ret = src;                    // Unnecessary
        src = ++p;
    }
    else
        src += strlen(src);

    return ret;
}

#endif /* __UDP_UDPSERIALIZE_H__ */
