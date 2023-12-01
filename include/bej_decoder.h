#pragma once

#include <stddef.h>

/**
 * \brief Decodes BEJ to JSON.
 *
 * \param dictionary Primary dictionary file.
 * \param bejFiles Input BEJ files.
 * \param fileCount Number of input BEJ files
 * \return 0 on success, 1 on failure.
 */
int bejDecode(char *dictionary, char **bejFiles, size_t fileCount);