#ifndef JSF_TYPE_DEFS_H_
#define JSF_TYPE_DEFS_H_

#ifndef Uint8
typedef unsigned char		Uint8;		/*!< unsigned 1 byte integer */
#endif
#ifndef Sint8
typedef char				Sint8;		/*!< signed 1 byte integer */
#endif
#ifndef Uint16
typedef unsigned short		Uint16;		/*!< unsigned 2 byte integer */
#endif
#ifndef Sint16
typedef short				Sint16;		/*!< signed 2 byte integer */
#endif
#ifndef Uint32
typedef unsigned int		Uint32;		/*!< unsigned 4 byte integer */
										/*typedef unsigned long		Uint32;*/
#endif
#ifndef Sint32
typedef signed int			Sint32;		/*!< signed 4 byte integer */
										/*typedef long				Sint32;*/
#endif

#ifndef ULong
typedef unsigned long		ULong;
#endif

#ifdef __LINUX__
typedef long long			Sint64;		/*!< unsigned 8 byte integer */
typedef unsigned long long	Uint64;		/*!< signed 8 byte integer */
#elif WIN32
typedef __int64				Sint64;
typedef unsigned __int64	Uint64;
#endif

#ifdef WIN32
#define JSF_SLEEP(A)	Sleep(A)
#elif __LINUX__
#define JSF_SLEEP(A)	usleep(A*1000)
#endif

enum JSFResult
{
	JSFRESULT_OK = 0,
	JSFRESULT_NG,
	JSFRESULT_ERR_UNSUPPORTED,
	JSFRESULT_ERR_ARGUMENT,
	JSFRESULT_SOCK_ERR_INVALIDSOCKET,
	JSFRESULT_SOCK_ERR_LISTEN,
	JSFRESULT_SOCK_ERR_ACCEPT,
	JSFRESULT_SOCK_ERR_CONNECT,
	JSFRESULT_SOCK_ERR_CONNECT_HOSTNAME,
	JSFRESULT_SOCK_ERR_SELECT,
	JSFRESULT_SOCK_ERR_BIND,
	JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK,
	JSFRESULT_SOCK_ERR_SEND,
	JSFRESULT_SOCK_ERR_SEND_CONNECT_CLOSE,
	JSFRESULT_SOCK_WRN_RECV_EWOULDBLOCK,
	JSFRESULT_SOCK_ERR_RECV,
	JSFRESULT_SOCK_ERR_RECV_CONNECT_CLOSE,
	JSFRESULT_SOCKEX_ERR_MEMORY_FULL,
	JSFRESULT_SOCKEX_WRN_WRITE_BUFFER,
	JSFRESULT_SOCKEX_WRN_READ_BUFFER,
	JSFRESULT_SOCKEX_ERR_WRITE,
	JSFRESULT_SOCKEX_ERR_READ,
	JSFRESULT_SOCKEX_ERR_BUFFER_OVERFLOW,
	JSFRESULT_SOCKMGR_ERR_MEMORY_FULL,
	JSFRESULT_SOCKMGR_ERR_POLL
};
#define	JSFRESULT_FAILED(CODE)	(CODE > JSFRESULT_OK)

#endif