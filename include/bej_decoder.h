#pragma once

#include <stddef.h>

/**
 *  @mainpage
 * \brief Decodes BEJ to JSON. Supported types: array, set, integer, string, boolean
 *
 * \param dictionary Primary dictionary file.
 * \param bejFiles Input BEJ files.
 * \param fileCount Number of input BEJ files
 * \return 0 on success, 1 on failure.
 */
int bejDecode(char *dictionary, char **bejFiles, size_t fileCount);