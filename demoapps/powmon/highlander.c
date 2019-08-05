/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Daniel Ellsworth <ellsworth8@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

/// Highlander... because there can be only one.

#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

static int amHighlander = -1;

static sem_t *semk = NULL;
static sem_t *seml = NULL;

/// @brief Determines/initializes the process highlander status.
int highlander(void)
{
    if (amHighlander != -1)
    {
        return amHighlander;
    }

    semk = sem_open("/power_wrapperK", O_CREAT | O_EXCL, 0600, 0);
    if(semk == NULL || semk == SEM_FAILED)
    {
        semk = sem_open("/power_wrapperK", O_CREAT, 0600, 0);
        seml = sem_open("/power_wrapperL", O_CREAT, 0600, 0);
        sem_post(semk);
        int v;
        sem_getvalue(semk, &v);
        amHighlander = 0;
        return 0;
    }
    seml = sem_open("/power_wrapperL", O_CREAT, 0600, 0);
    amHighlander = 1;
    return 1;
}

int highlander_clean(void)
{
    // remove the named semaphores
    fprintf(stdout, "Removing named semaphore /power_wrapperK\n");
    sem_unlink("/power_wrapperK");
    fprintf(stdout, "Removing named semaphore /power_wrapperL\n");
    sem_unlink("/power_wrapperL");
    return 1;
}

/// @brief Causes the highlander to wait until all foes have called wait.
int highlander_wait(void)
{
    if (amHighlander < 0)
    {
        return 1;
    }
    if (amHighlander)
    {
        int v;
        sem_getvalue(semk, &v);
        while (v > 1)
        {
            sem_wait(seml);
        }

        sem_close(semk);
        sem_close(seml);
        sem_unlink("/power_wrapperK");
        sem_unlink("/power_wrapperL");

        return 0;
    }
    else
    {
        sem_wait(semk);
        sem_post(seml);
        sem_close(semk);
        sem_close(seml);
        return 0;
    }
    return 1;
}
