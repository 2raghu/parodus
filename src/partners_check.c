/**
 * Copyright 2015 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
/**
 * @file partners_check.c
 *
 * @description This describes functions to validate partner_id.
 *
 */

#include "ParodusInternal.h"
#include "config.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             Internal Functions                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             External functions                             */
/*----------------------------------------------------------------------------*/
static void parse_partner_id(char *partnerId, partners_t **partnersList)
{
    char *token;
    int i = 0, j = 0, count = 0;
    ParodusPrint("********* %s ********\n",__FUNCTION__);
    while(partnerId[i] != '\0')
    {
        if(partnerId[i] == ',')
        {
            count++;
        }
        i++;
    }
    ParodusPrint("count = %d\n", count+1);
    *partnersList = (partners_t *)malloc(sizeof(partners_t)+ sizeof( char * ) * (count+1));
    memset(*partnersList, 0, sizeof(partners_t));
    (*partnersList)->count = count+1;
    while ((token = strsep(&partnerId, ",")) != NULL)
    {
        ParodusInfo("token=%s\n", token);
        (*partnersList)->partner_ids[j] = strdup(token);
        ParodusInfo("(*partnersList)->partner_ids[%d] = %s\n",j,(*partnersList)->partner_ids[j]);
        j++;
    }
}

int validate_partner_id(wrp_msg_t *msg, partners_t **partnerIds)
{
    int matchFlag = 0, i = 0, count = 0;
    size_t j = 0;
    partners_t *partnersList = NULL;
    char *partnerId = NULL;
    ParodusPrint("********* %s ********\n",__FUNCTION__);
    ParodusInfo("********* %s ********\n",__FUNCTION__);
    char *temp = get_parodus_cfg()->partner_id;
    ParodusInfo("temp = %s\n",temp);
    if(temp[0] != '\0' && strlen(temp) > 0)
    {
        partnerId = strdup(temp);
    }
    ParodusInfo("partnerId = %s\n",partnerId);
    if(partnerId != NULL)
    {
        parse_partner_id(partnerId, &partnersList);
        ParodusInfo("partnersList->count = %lu\n", partnersList->count);
        if(msg->msg_type == WRP_MSG_TYPE__EVENT)
        {
            if(msg->u.event.partner_ids != NULL)
            {
                count = (int) msg->u.event.partner_ids->count;
                ParodusInfo("EVE:partner_ids count is %d\n",count);
                for(i = 0; i < count; i++)
                {
                    for(j = 0; j<partnersList->count; j++)
                    {
			if(NULL != partnersList->partner_ids[j]) {
		                ParodusInfo("EVE:partnersList->partner_ids[%lu] = %s\n",j, partnersList->partner_ids[j]);
				ParodusInfo("EVE:msg->u.event.partner_ids->partner_ids[%lu] = %s\n",i, msg->u.event.partner_ids->partner_ids[i]);
		                if(strcasecmp(partnersList->partner_ids[j], msg->u.event.partner_ids->partner_ids[i]) == 0)
		                {
		                    ParodusInfo("EVE: partner_id match found\n");
		                    matchFlag = 1;
		                    break;
		                }
			}
			else
				ParodusError("EVE:partner Id in partnersList is NULL but count is not 0");
                    }
                    /* Commandline input partner_ids matched with partner_ids from request */
                    if(matchFlag == 1)
                    {
                        break;
                    }
                }

                /* Commandline input partner_ids not matching with partner_ids from request, appending to request partner_ids*/
                if(matchFlag != 1)
                {
                    (*partnerIds) = (partners_t *) malloc(sizeof(partners_t) + (sizeof(char *) * (count+partnersList->count)));
                    (*partnerIds)->count = count+partnersList->count;
                    for(i = 0; i < count; i++)
                    {
                        (*partnerIds)->partner_ids[i] = msg->u.event.partner_ids->partner_ids[i];
                        ParodusInfo("EVE: (*partnerIds)->partner_ids[%d] : %s\n",i,(*partnerIds)->partner_ids[i]);
                    }
                    i = 0;
                    for(j = count; j<(count+partnersList->count); j++)
                    {
                        (*partnerIds)->partner_ids[j] = (char *) malloc(sizeof(char) * 64);
                        parStrncpy((*partnerIds)->partner_ids[j], partnersList->partner_ids[i], 64);
                        ParodusInfo("EVE:: (*partnerIds)->partner_ids[%lu] : %s\n",j,(*partnerIds)->partner_ids[j]);
                        i++;
                    }
                }
            }
            else
            {
                ParodusInfo("EVE:partner_ids list is NULL\n");
                (*partnerIds) = (partners_t *) malloc(sizeof(partners_t) + (sizeof(char *) * partnersList->count));
                (*partnerIds)->count = partnersList->count;
                i=0;
                for(j = 0; j<partnersList->count; j++)
                {
                    (*partnerIds)->partner_ids[j] = (char *) malloc(sizeof(char) * 64);
                    parStrncpy((*partnerIds)->partner_ids[j], partnersList->partner_ids[i], 64);
                    ParodusInfo("EVE:- (*partnerIds)->partner_ids[%lu] : %s\n",j,(*partnerIds)->partner_ids[j]);
                    i++;
                }
            }
        }
        else if(msg->msg_type == WRP_MSG_TYPE__REQ)
        {
            if(msg->u.req.partner_ids != NULL)
            {
                count = (int) msg->u.req.partner_ids->count;
                ParodusInfo("REQ:partner_ids count is %d\n",count);
                for(i = 0; i < count; i++)
                {
                    for(j = 0; j<partnersList->count; j++)
                    {
			if(NULL != partnersList->partner_ids[j]) 
			{
		                ParodusInfo("REQ:partnersList->partner_ids[%lu] = %s\n",j, partnersList->partner_ids[j]);
				ParodusInfo("msg->u.req.partner_ids->partner_ids[%lu] = %s\n",i, msg->u.req.partner_ids->partner_ids[i]);
		                if(strcasecmp(partnersList->partner_ids[j], msg->u.req.partner_ids->partner_ids[i]) == 0)
		                {
		                    ParodusInfo("REQ: partner_id match found\n");
		                    matchFlag = 1;
		                    break;
		                }
			}
			else
				ParodusError("partner Id in partnersList is NULL but count is not 0");
                    }
                }

                /* Commandline input partner_ids not matching with partner_ids from request, ignoring request*/
                if(matchFlag != 1)
                {
                    ParodusError("Invalid partner_id %s\n",temp);
                    OnboardLog("Invalid partner_id %s\n",temp);
                    if(partnersList != NULL)
                    {
                        for(j=0; j<partnersList->count; j++)
                        {
                            if(NULL != partnersList->partner_ids[j])
                            {
                                free(partnersList->partner_ids[j]);
                            }
                        }
                        free(partnersList);
                    }
                    free(partnerId);
                    return -1;
                }
            }
            else
            {
                ParodusInfo("REQ:partner_ids list is NULL\n");
            }
        }
        if(partnersList != NULL)
        {
            for(j=0; j<partnersList->count; j++)
            {
                if(NULL != partnersList->partner_ids[j])
                {
                    free(partnersList->partner_ids[j]);
                }
            }
            free(partnersList);
        }
        free(partnerId);
    }
    else
    {
        ParodusInfo("partner_id is not available to validate\n");
        return 0;
    }
    return 1;
}

