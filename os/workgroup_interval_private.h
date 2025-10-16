#ifndef __OS_WORKGROUP_INTERVAL_PRIVATE__
#define __OS_WORKGROUP_INTERVAL_PRIVATE__

#ifndef __OS_WORKGROUP_PRIVATE_INDIRECT__
#error "Please #include <os/workgroup_private.h> instead of this file directly."
#include <os/workgroup_base.h> // For header doc
#endif

__BEGIN_DECLS

OS_WORKGROUP_ASSUME_NONNULL_BEGIN

/*
 * @typedef os_workgroup_interval_type_t
 *
 * @abstract
 * Describes a specialized os_workgroup_interval type the client would like to
 * create.
 *
 * Clients need the 'com.apple.private.kernel.work-interval' entitlement to
 * create all workgroups types listed below except the following:
 *
 * OS_WORKGROUP_INTERVAL_TYPE_DEFAULT,
 * OS_WORKGROUP_INTERVAL_TYPE_CA_CLIENT,
 * OS_WORKGROUP_INTERVAL_TYPE_AUDIO_CLIENT,
 *
 * Note that only real time threads are allowed to join workgroups of type
 * OS_WORKGROUP_INTERVAL_TYPE_AUDIO_CLIENT and
 * OS_WORKGROUP_INTERVAL_TYPE_COREAUDIO.
 */
OS_ENUM(os_workgroup_interval_type, uint16_t,
	OS_WORKGROUP_INTERVAL_TYPE_DEFAULT = 0x1,
	OS_WORKGROUP_INTERVAL_TYPE_CA_CLIENT,
	OS_WORKGROUP_INTERVAL_TYPE_AUDIO_CLIENT,

	OS_WORKGROUP_INTERVAL_TYPE_COREAUDIO,
	OS_WORKGROUP_INTERVAL_TYPE_COREANIMATION,
	OS_WORKGROUP_INTERVAL_TYPE_CA_RENDER_SERVER,
	OS_WORKGROUP_INTERVAL_TYPE_HID_DELIVERY,
	OS_WORKGROUP_INTERVAL_TYPE_COREMEDIA,

	OS_WORKGROUP_INTERVAL_TYPE_ARKIT,
	OS_WORKGROUP_INTERVAL_TYPE_FRAME_COMPOSITOR,
);

/*
 * @function os_workgroup_attr_set_interval_type
 *
 * @abstract
 * Specifies that the os_workgroup_interval_t to be created should be of a
 * specialized type. These types should only be specified when creating an
 * os_workgroup_interval_t using the os_workgroup_interval_create or
 * os_workgroup_interval_create_with_workload_id APIs - using it with any other
 * workgroup creation API will result in an error at creation time.
 *
 * When used with os_workgroup_interval_create_with_workload_id, the type
 * specified via this attribute must match the one configured by the system for
 * the provided workload identifier (if that identifier is known).
 *
 * Setting type OS_WORKGROUP_INTERVAL_TYPE_DEFAULT on an os_workgroup_interval_t
 * is a no-op.
 *
 * EINVAL is returned if the attribute passed in hasn't been initialized.
 */
API_AVAILABLE(macos(10.16), ios(14.0), tvos(14.0), watchos(7.0))
OS_WORKGROUP_EXPORT
int
os_workgroup_attr_set_interval_type(os_workgroup_attr_t attr,
		os_workgroup_interval_type_t type);

/*!
 * @enum os_workgroup_telemetry_flavor_t
 *
 * @abstract
 * Flavors of os_workgroup telemetry that can be queried, each
 * corresponding to a telemetry structure type.
 */
OS_ENUM(os_workgroup_telemetry_flavor, uint16_t,
	/*!
	 * @const OS_WORKGROUP_TELEMETRY_FLAVOR_BASIC
	 * Telemetry as specified per the fields in os_workgroup_telemetry_basic_s
	 */
	OS_WORKGROUP_TELEMETRY_FLAVOR_BASIC = 1,
);

/*!
 * @function os_workgroup_attr_set_telemetry_flavor
 *
 * @abstract
 * Sets a telemetry flavor in the workgroup attribute which will determine
 * the type of telemetry structure that the workgroup can query data for.
 *
 * Returns EINVAL if the telemetry flavor is invalid or if the workgroup
 * attribute was not correctly initialized.
 */
SPI_AVAILABLE(macos(14.0), ios(17.0), tvos(17.0), watchos(10.0))
OS_WORKGROUP_EXPORT OS_WORKGROUP_WARN_RESULT
int
os_workgroup_attr_set_telemetry_flavor(os_workgroup_attr_t wga,
		os_workgroup_telemetry_flavor_t flavor);

/*
 * @typedef os_workgroup_interval_data_flags_t
 *
 * @abstract
 * Set of flags that can be passed to os_workgroup_interval_data_set_flags() to
 * configure calls to os_workgroup_interval_start(),
 * os_workgroup_interval_update() and os_workgroup_interval_finish() to
 * indicate to the system that a specific instance of a repeatable workload
 * has one of the following properties:
 */
OS_OPTIONS(os_workgroup_interval_data_flags, uint32_t,
	OS_WORKGROUP_INTERVAL_DATA_NONE = 0xffffffff,
	OS_WORKGROUP_INTERVAL_DATA_COMPLEXITY_DEFAULT OS_WORKGROUP_ENUM_API_DEPRECATED_WITH_REPLACEMENT(
			"Use os_workgroup_interval_data_set_complexity instead",
			macos(13.0,16.0), ios(16.0,19.0), tvos(16.0,19.0), watchos(9.0,12.0)) = 0x0u,
	OS_WORKGROUP_INTERVAL_DATA_COMPLEXITY_HIGH OS_WORKGROUP_ENUM_API_DEPRECATED_WITH_REPLACEMENT(
			"Use os_workgroup_interval_data_set_complexity instead",
			macos(13.0,16.0), ios(16.0,19.0), tvos(16.0,19.0), watchos(9.0,12.0)) = 0x1u,
);

/*
 * @function os_workgroup_interval_data_set_flags
 *
 * @abstract
 * Setter for os_workgroup_interval_data_t, can specify an ORd combination of
 * flags from os_workgroup_interval_data_flags_t to indicate to the system that
 * a specific instance of a repeatable workload has custom properties by
 * passing the resulting data pointer to os_workgroup_interval_start(),
 * os_workgroup_interval_update() and os_workgroup_interval_finish().
 *
 * The flags chosen for a given instance of the repeatable workload are allowed
 * to be different at each of these three calls made for the instance, and they
 * are determined wholly by the `data` value passed to the specific call.
 *
 * @discussion
 * In particular this means that once a non-default flag is set with this
 * function, the resulting data pointer must be passed to every subsequent
 * call of update() or finish() for that instance if the goal is to keep that
 * flag present (as opposed to e.g. being reset to the default by passing a
 * NULL data pointer).
 *
 * @param data
 * Pointer to workgroup interval data structure initialized with
 * OS_WORKGROUP_INTERVAL_DATA_INITIALIZER.
 *
 * @param flags
 * ORd combination of flags from os_workgroup_interval_data_flags_t to set in
 * the specified interval data.
 *
 * @result
 * EINVAL is returned if the interval data passed in hasn't been initialized,
 * or if unknown or an invalid combination of flag values are passed.
 */
API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(9.0))
OS_WORKGROUP_EXPORT
int
os_workgroup_interval_data_set_flags(os_workgroup_interval_data_t data,
		os_workgroup_interval_data_flags_t flags);

// TODO: rdar://145714553, remove revlock avoiding flag
#define __OS_WORKGROUP_INTERVAL_DATA_HAS_CUSTOM_COMPLEXITY 1

/*
 * @typedef os_workgroup_interval_data_complexity_t
 *
 * @abstract
 * Set of flags that can be passed to os_workgroup_interval_data_set_complexity() to
 * indicate to the system that a specific instance of a repeatable workload
 * has one of the following properties:
 *
 * OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_DEFAULT:
 *     specific instance has default complexity (same as using data initialized
 *     with OS_WORKGROUP_INTERVAL_DATA_INITIALIZER resp not calling
 *     os_workgroup_interval_data_set_flags(), or specifying NULL
 *     os_workgroup_interval_data_t).
 * OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_HIGH:
 *     specific instance has high complexity. May only be called on an
 *     os_workgroup_interval_t created with a workload identifier that is known
 *     and is configured by the system to be allowed to use complexity.
 * OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_MANUAL:
 *     specific instance has complexity that is being manually signalled by
 *	   the client. May only be called on an os_workgroup_interval_t created
 * 	   with a workload identifier that is known and is configured by the system
 *	   to be allowed to use complexity.The value interpretation is work interval
 *     type specific.
 */
OS_OPTIONS(os_workgroup_interval_data_complexity, uint32_t,
	OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_DEFAULT = 0x0u,
	OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_HIGH = 0x1u,
	OS_WORKGROUP_INTERVAL_DATA_SET_COMPLEXITY_MANUAL = 0x2u,
);

/*
 * @function os_workgroup_interval_data_set_complexity
 *
 * @abstract
 * Setter for os_workgroup_interval_data_t, can specify a complexity value
 * to indicate to the system that a specific instance of a repeatable workload
 * has custom properties by passing the resulting data pointer to
 * os_workgroup_interval_start(), os_workgroup_interval_update() and
 * os_workgroup_interval_finish(). This value will be ignored if any
 * non-default complexity flags have been set using
 * os_workgroup_interval_data_set_flags().
 *
 * The complexity chosen for a given instance of the repeatable workload are allowed
 * to be different at each of these three calls made for the instance, and they
 * are determined wholly by the `data` value passed to the specific call.
 *
 * @discussion
 * In particular this means that once a non-default flag is set with this
 * function, the resulting data pointer must be passed to every subsequent
 * call of update() or finish() for that instance if the goal is to keep that
 * flag present (as opposed to e.g. being reset to the default by passing a
 * NULL data pointer).
 *
 * The interpretation of this complexity value is specific to a work interval type.
 *
 * @param data
 * Pointer to workgroup interval data structure initialized with
 * OS_WORKGROUP_INTERVAL_DATA_INITIALIZER.
 *
 * @param complexity
 * Work interval type specific complexity value.
 *
 */
API_AVAILABLE(macos(16.0), ios(19.0), tvos(19.0), watchos(12.0))
OS_WORKGROUP_EXPORT
int
os_workgroup_interval_data_set_complexity(os_workgroup_interval_data_t data,
		os_workgroup_interval_data_complexity_t flag, uint64_t complexity);

/*!
 * @function os_workgroup_interval_data_set_flags
 *
 * @abstract
 * Setter for os_workgroup_interval_data_t that specifies a telemetry flavor and
 * a pointer to a telemetry structure of the corresponding flavor where telemetry
 * data should be written out. Telemetry data will be written out to the pointer on
 * a successful call to os_workgroup_interval_start(),
 * os_workgroup_interval_update(), or os_workgroup_interval_finish() where the
 * os_workgroup_interval_data_t was passed as a parameter.
 *
 * @param data
 * Pointer to workgroup interval data structure initialized with
 * OS_WORKGROUP_INTERVAL_DATA_INITIALIZER.
 *
 * @param flavor
 * Specifies the kind of the telemetry to be retrieved.
 *
 * @param telemetry
 * Pointer to a telemetry struct of the specified flavor where telemetry data
 * should be written.
 *
 * @param size
 * Size of the telemetry struct where telemetry data should be written.
 *
 * @result
 * EINVAL is returned if the interval data passed in hasn't been initialized
 * or if an unknown or invalid combination of flavor and size values are passed.
 */
SPI_AVAILABLE(macos(14.0), ios(17.0), tvos(17.0), watchos(10.0))
OS_WORKGROUP_EXPORT
int
os_workgroup_interval_data_set_telemetry(os_workgroup_interval_data_t data,
		os_workgroup_telemetry_flavor_t flavor, void *telemetry, size_t size);

/*!
 * @struct os_workgroup_telemetry_basic_s
 *
 * @abstract
 * A structure containing telemetry data for a workgroup. Fields are cumulative and
 * reflect aggregate statistics from threads while they are joined to the workgroup.
 *
 * @field wg_external_wakeups
 * Number of times a thread joined to the workgroup was woken up by an "external"
 * thread not joined to the workgroup.
 *
 * @field wg_total_wakeups
 * Number of times a thread joined to the workgroup blocked and was woken up.
 *
 * @field wg_user_time_mach
 * Time in Mach units that threads joined to the workgroup spent on-core running
 * in user-space.
 *
 * @field wg_system_time_mach
 * Time in Mach units that threads joined to the workgroup spent on-core running
 * in the kernel.
 *
 * @field wg_cycles
 * Number of CPU cycles consumed by threads in the workgroup. Always set to 0 if not
 * supported by the underlying hardware.
 *
 * @field wg_instructions
 * Number of instructions executed by threads in the workgroup. Always set to 0 if not
 * supported by the underlying hardware.
 */
struct os_workgroup_telemetry_basic_s {
	uint32_t        wg_external_wakeups;
	uint32_t        wg_total_wakeups;
	uint64_t        wg_user_time_mach;
	uint64_t        wg_system_time_mach;
	uint64_t        wg_cycles;
	uint64_t        wg_instructions;
};

/*!
 * @typedef os_workgroup_telemetry_basic, os_workgroup_telemetry_basic_t
 *
 * @abstract
 * A structure containing basic telemetry data fields which represent aggregate
 * statistics for the workgroup.
 */
typedef struct os_workgroup_telemetry_basic_s os_workgroup_telemetry_basic_s;
typedef struct os_workgroup_telemetry_basic_s *os_workgroup_telemetry_basic_t;
#define OS_WORKGROUP_TELEMETRY_INITIALIZER { 0 }

/*
 * @function os_workgroup_interval_create
 *
 * @abstract
 * Creates an os_workgroup_interval_t with the specified name and attributes.
 * This object tracks a repeatable workload characterized by a start time, end
 * time and targeted deadline. Example use cases include audio and graphics
 * rendering workloads.
 *
 * A newly created os_workgroup_interval_t has no initial member threads - in
 * particular the creating thread does not join the os_workgroup_interval_t
 * implicitly.
 *
 * @param name
 * A client specified string for labelling the workgroup. This parameter is
 * optional and can be NULL.
 *
 * @param clockid
 * The clockid in which timestamps passed to the os_workgroup_interval_start()
 * and os_workgroup_interval_update() functions are specified.
 *
 * @param attrs
 * The requested set of os_workgroup_t attributes. NULL is to be specified for
 * the default set of attributes. By default, an interval workgroup
 * is nonpropagating with asynchronous work and differentiated from other threads
 * in the process (see os_workgroup_attr_flags_t).
 *
 * The OS_WORKGROUP_ATTR_UNDIFFERENTIATED attribute is invalid to specify for
 * interval workgroups. If it isn't or if invalid attributes are specified, this
 * function returns NULL and sets errno.
 */
API_AVAILABLE(macos(10.16), ios(14.0), tvos(14.0), watchos(7.0))
OS_WORKGROUP_EXPORT OS_WORKGROUP_RETURNS_RETAINED
os_workgroup_interval_t _Nullable
os_workgroup_interval_create(const char * _Nullable name, os_clockid_t clock,
		os_workgroup_attr_t _Nullable attr);

/*
 * @function os_workgroup_interval_create_with_workload_id
 *
 * @abstract
 * Creates an os_workgroup_interval_t with the specified name and workload
 * identifier.
 * This object tracks a repeatable workload characterized by a start time, end
 * time and targeted deadline. Example use cases include audio and graphics
 * rendering workloads.
 *
 * The newly created os_workgroup_interval_t has no initial member threads - in
 * particular the creating thread does not join the os_workgroup_interval_t
 * implicitly.
 *
 * @param name
 * A client specified string for labelling the workgroup. This parameter is
 * optional and can be NULL.
 *
 * @param workload_id
 * A system-defined workload identifier string determining the configuration
 * parameters to apply to the workgroup and its member threads.
 * Must not be NULL.
 * If the specified identifier is known, it must refer to a workload configured
 * as being of interval type, or this function will return NULL.
 * See discussion for the detailed rules used to combine the information
 * specified by the `workload_id` and `wga` arguments.
 *
 * @param clockid
 * The clockid in which timestamps passed to the os_workgroup_interval_start()
 * and os_workgroup_interval_update() functions are specified.
 *
 * @param wga
 * The requested set of os_workgroup_t attributes. NULL is to be specified for
 * the default set of attributes. By default, a workgroup created with workload
 * identifier is nonpropagating with asynchronous work and differentiated from
 * other threads in the process (see os_workgroup_attr_flags_t).
 * The interval type specified by the attributes will be used as a fallback in
 * case the provided workload identifier is unknown.
 * Any telemetry flavor specified by the attributes will also be used,
 * regardless of whether the workload identifier is known.
 * See discussion for the detailed rules used to combine the information
 * specified by the `workload_id` and `wga` arguments.
 *
 * @discussion
 * Rules used for resolution of configuration parameters potentially specified
 * by both workload identifier and attributes, applied in order:
 * - If the provided attributes are NULL or equal to the default set of
 *   attributes, no parameters are considered to be explicitly specified via
 *   attribute.
 * - If the provided workload identifier is known, and the provided attributes
 *   explicitly specify a parameter that is also configured by the identifier,
 *   the two parameter values must match or this function will fail and return
 *   an error.
 * - If the provided workload identifier is known, the parameters configured by
 *   the identifier will be used.
 * - If the provided workload identifier is unknown, the parameters specified
 *   via the provided attributes will be used as a fallback.
 * - If a given parameter is neither configured by a known workload identifier
 *   or explicitly specified via an attribute, a system-dependent fallback
 *   value will be used.
 *
 * @result
 * The newly created workgroup object, or NULL if invalid arguments were
 * specified (in which case errno is also set).
 */
SPI_AVAILABLE(macos(12.0), ios(15.0), tvos(15.0), watchos(8.0))
OS_WORKGROUP_EXPORT OS_WORKGROUP_RETURNS_RETAINED
os_workgroup_interval_t _Nullable
os_workgroup_interval_create_with_workload_id(const char * _Nullable name,
		const char *workload_id, os_clockid_t clock,
		os_workgroup_attr_t _Nullable attr);

/* This SPI is for use by Audio Toolbox only. This function returns a reference
 * which is the responsibility of the caller to manage.
 */
API_AVAILABLE(macos(10.16), ios(14.0), tvos(14.0), watchos(7.0))
OS_WORKGROUP_EXPORT OS_WORKGROUP_RETURNS_RETAINED
os_workgroup_t _Nullable
os_workgroup_interval_copy_current_4AudioToolbox(void);

OS_WORKGROUP_ASSUME_NONNULL_END

__END_DECLS
#endif /* __OS_WORKGROUP_INTERVAL_PRIVATE__ */
