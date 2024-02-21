#ifndef CROWD_CONTROL_VIRTUALS_HPP
#define CROWD_CONTROL_VIRTUALS_HPP

/* Workaround for virtual functions
 *
 * Define a function _<virtual> and bind it with BIND_VIRTUAL_METHOD
 * Then whenever you need to call the function use these macros.
 * NOTE: BIND_VIRTUAL_METHOD does NOT expose the function in the documentation.
 * Exposing it manually breaks GDScript support as builtin functions cannot
 * be overridden.
 */

#define VIRTUAL_CALL(m_method) \
	call(#m_method)

#define VIRTUAL_CALL_ARGS(m_method, ...) \
	call(#m_method, __VA_ARGS__)

#define VIRTUAL_CALL_PTR(m_object, m_method) \
	m_object->call(#m_method)

#define VIRTUAL_CALL_PTR_ARGS(m_method, ...) \
	m_object->call(#m_method, __VA_ARGS__)

#endif // CROWD_CONTROL_VIRTUALS_HPP