/**
 * Copyright (c) 2023 Hemashushu <hippospark@gmail.com>, All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Open device file `/dev/charone` and read 10 chars and sleep 10 seconds, then exit.
 *
 */
int main(void)
{
    char buf[10];
    FILE *file = fopen("/dev/charone", "r");
    if (file == NULL)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fread(buf, 1, 10, file);

    buf[9] = '\0';
    printf("read chars: %s\n", buf);
    sleep(10);
    fclose(file);

    return EXIT_SUCCESS;
}