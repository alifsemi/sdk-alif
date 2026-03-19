/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef CGMS_APP_H
#define CGMS_APP_H

void server_configure(void);
void disc_notify(uint16_t reason);

/* Service specific functions */
void service_conn_cgms(struct shared_control *ctrl);
void cgms_process(void);
void addr_res_done(void);
void cgms_record_store_size_update(void);

#endif /* CGMS_APP_H */
