#pragma once

#define SINGLETON_IMPL(T) T* T::m_pInstance = NULL;
#define SINGLETON_DECL(T) public: \
	static T* Get() { return m_pInstance; } \
	static void Create() { m_pInstance = new T(); } \
	static void Destroy() { SAFE_DELETE(m_pInstance); } \
protected: static T* m_pInstance; \
	T(void); ~T(void);

