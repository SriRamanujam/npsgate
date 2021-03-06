/*
 *    Copyright 2004-2006 Intel Corporation
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef DTN_API_H
#define DTN_API_H

#include "dtn_errno.h"
#include "dtn_types.h"

#define INSTALL_LOCALSTATEDIR "/tmp" /* XXX hack */

/**
 * The basic handle for communication with the dtn router.
 */
typedef int* dtn_handle_t;

#ifdef  __cplusplus
extern "C" {
#endif

/*************************************************************
 *
 *                     API Functions
 *
 *************************************************************/

/**
 * Open a new connection to the router.
 *
 * On success, initializes the handle parameter as a new handle to the
 * daemon and returns DTN_SUCCESS. On failure, sets handle to NULL and
 * returns a dtn_errno error code.
 */
extern int dtn_open(dtn_handle_t* handle);

/**
 * Close an open dtn handle.  Returns DTN_SUCCESS on success.
 */
extern int dtn_close(dtn_handle_t handle);

/**
 * Get the error associated with the given handle.
 */
extern int dtn_errno(dtn_handle_t handle);

/**
 * Set the error associated with the given handle
 */
extern void dtn_set_errno(dtn_handle_t handle, int err);

/**
 * Build an appropriate local endpoint id by appending the specified
 * service tag to the daemon's preferred administrative endpoint id.
 */
extern int dtn_build_local_eid(dtn_handle_t handle,
                               dtn_endpoint_id_t* local_eid,
                               const char* service_tag);

/**
 * Create a dtn registration. If the init_passive flag in the reginfo
 * struct is true, the registration is initially in passive state,
 * requiring a call to dtn_bind to activate it. Otherwise, the call to
 * dtn_bind is unnecessary as the registration will be created in the
 * active state and bound to the handle.
 */
extern int dtn_register(dtn_handle_t handle,
                        dtn_reg_info_t* reginfo,
                        dtn_reg_id_t* newregid);

/**
 * Remove a dtn registration.
 *
 * If the registration is in the passive state (i.e. not bound to any
 * handle), it is immediately removed from the system. If it is in
 * active state and bound to the given handle, the removal is deferred
 * until the handle unbinds the registration or closes. This allows
 * applications to pre-emptively call unregister so they don't leak
 * registrations.
 */
extern int dtn_unregister(dtn_handle_t handle,
                          dtn_reg_id_t regid);

/**
 * Check for an existing registration on the given endpoint id,
 * returning DTN_SUCCSS and filling in the registration id if it
 * exists, or returning ENOENT if it doesn't.
 */
extern int dtn_find_registration(dtn_handle_t handle,
                                 dtn_endpoint_id_t* eid,
                                 dtn_reg_id_t* newregid);

/**
 * Modify an existing registration.
 */
extern int dtn_change_registration(dtn_handle_t handle,
                                   dtn_reg_id_t regid,
                                   dtn_reg_info_t *reginfo);

/**
 * Associate a registration with the current ipc channel.
 * This serves to put the registration in "active" mode.
 */
extern int dtn_bind(dtn_handle_t handle, dtn_reg_id_t regid);

/**
 * Explicitly remove an association from the current ipc handle. Note
 * that this is also implicitly done when the handle is closed.
 *
 * This serves to put the registration back in "passive" mode.
 */
extern int dtn_unbind(dtn_handle_t handle, dtn_reg_id_t regid);

/**
 * Send a bundle either from memory or from a file.
 */
extern int dtn_send(dtn_handle_t handle,
                    dtn_reg_id_t regid,
                    dtn_bundle_spec_t* spec,
                    dtn_bundle_payload_t* payload,
                    dtn_bundle_id_t* id);

/**
 * Cancel a bundle transmission.
 */
extern int dtn_cancel(dtn_handle_t handle,
                      dtn_bundle_id_t* id);

/**
 * Blocking receive for a bundle, filling in the spec and payload
 * structures with the bundle data. The location parameter indicates
 * the manner by which the caller wants to receive payload data (i.e.
 * either in memory or in a file). The timeout parameter specifies an
 * interval in milliseconds to block on the server-side (-1 means
 * infinite wait).
 *
 * Note that it is advisable to call dtn_free_payload on the returned
 * structure, otherwise the XDR routines will memory leak.
 */
extern int dtn_recv(dtn_handle_t handle,
                    dtn_bundle_spec_t* spec,
                    dtn_bundle_payload_location_t location,
                    dtn_bundle_payload_t* payload,
                    dtn_timeval_t timeout);

/**
 * Blocking query for new subscribers on a session. One or more
 * registrations must have been bound to the handle with the
 * SESSION_CUSTODY flag set. Returns an indication that the
 * subscription state in the given session has changed (i.e. a
 * subscriber was added or removed).
 */
extern int dtn_session_update(dtn_handle_t handle,
                              unsigned int* status,
                              dtn_endpoint_id_t* session,
                              dtn_timeval_t timeout);

/**
 * Return a file descriptor for the given handle suitable for calling
 * poll() or select() in conjunction with a call to dtn_begin_poll().
 */
extern int dtn_poll_fd(dtn_handle_t handle);

/**
 * Begin a polling period for incoming bundles. Returns a file
 * descriptor suitable for calling poll() or select() on (similar to
 * dtn_poll_fd). Note that dtn_bind() must have been previously called
 * at least once on the handle.
 *
 * If the kernel returns an indication that there is data ready on the
 * file descriptor, a call to dtn_recv will then return the bundle
 * without blocking, then dtn_poll must be called again to wait for
 * more bundles.
 *
 * Also, no other API calls besides dtn_recv can be executed during a
 * polling period, so an app must call dtn_cancel_poll before trying
 * to run other API calls.
 */
extern int dtn_begin_poll(dtn_handle_t handle, dtn_timeval_t timeout);

/**
 * Cancel a polling interval.
 */
extern int dtn_cancel_poll(dtn_handle_t handle);

/*************************************************************
 *
 *                     Utility Functions
 *
 *************************************************************/

/**
 * Copy the contents of one eid into another.
 */
extern void dtn_copy_eid(dtn_endpoint_id_t* dst, dtn_endpoint_id_t* src);

/**
 * Parse a string into an endpoint id structure, validating that it is
 * in fact a valid endpoint id (i.e. a URI).
 */
extern int dtn_parse_eid_string(dtn_endpoint_id_t* eid, const char* str);

/**
 * Sets the value of the given payload structure to either a memory
 * buffer or a file location.
 *
 * Returns: 0 on success, DTN_ESIZE if the memory location is
 * selected and the payload is too big.
 */
extern int dtn_set_payload(dtn_bundle_payload_t* payload,
                           dtn_bundle_payload_location_t location,
                           char* val, int len);

/**
 * Frees dynamic storage allocated by the xdr for a bundle payload in
 * dtn_recv.
 */
void dtn_free_payload(dtn_bundle_payload_t* payload);

/**
 * Return a string version of a status report reason code.
 */
const char* dtn_status_report_reason_to_str(dtn_status_report_reason_t err);

#ifdef  __cplusplus
}
#endif

#endif /* DTN_API_H */
