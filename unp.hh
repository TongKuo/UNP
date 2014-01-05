///:
/*****************************************************************************
 **                                                                         **
 **                               .======.                                  **
 **                               | INRI |                                  **
 **                               |      |                                  **
 **                               |      |                                  **
 **                      .========'      '========.                         **
 **                      |   _      xxxx      _   |                         **
 **                      |  /_;-.__ / _\  _.-;_\  |                         **
 **                      |     `-._`'`_/'`.-'     |                         **
 **                      '========.`\   /`========'                         **
 **                               | |  / |                                  **
 **                               |/-.(  |                                  **
 **                               |\_._\ |                                  **
 **                               | \ \`;|                                  **
 **                               |  > |/|                                  **
 **                               | / // |                                  **
 **                               | |//  |                                  **
 **                               | \(\  |                                  **
 **                               |  ``  |                                  **
 **                               |      |                                  **
 **                               |      |                                  **
 **                               |      |                                  **
 **                               |      |                                  **
 **                   \\    _  _\\| \//  |//_   _ \// _                     **
 **                  ^ `^`^ ^`` `^ ^` ``^^`  `^^` `^ `^                     **
 **                                                                         **
 **                    Copyright © 1997-2014 by Tong G.                     **
 **                          ALL RIGHTS RESERVED.                           **
 **                                                                         **
 ****************************************************************************/

#ifndef __UNP_HH_INCLUDED__
#define __UNP_HH_INCLUDED__

#include    <iostream>
#include    <cstdio>
#include    <string>
#include    <cstring>
#include    <cstdlib>
#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */

size_t static const MAXLINE = 100;
size_t static const MAXPENDING = 5;

typedef struct sockaddr __SA;

#include	<errno.h>		/* for definition of errno */
#include	<stdarg.h>		/* ANSI C header file */

static void	_ErrDoit(int, const char *, va_list);

char	*pname = NULL;		/* caller can set this from argv[0] */

/* Nonfatal error related to a system call.
 * Print a message and return. */

void
/* $f err_ret $ */
_ErrRet(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    _ErrDoit(1, fmt, ap);
    va_end(ap);
    return;
}

/* Fatal error related to a system call.
 * Print a message and terminate. */

void
/* $f err_sys $ */
_ErrSys(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    _ErrDoit(1, fmt, ap);
    va_end(ap);
    exit(1);
}

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */

void
/* $f err_dump $ */
_ErrDump(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    _ErrDoit(1, fmt, ap);
    va_end(ap);
    abort();		/* dump core and terminate */
    exit(1);		/* shouldn't get here */
}

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */

void
/* $f err_msg $ */
_ErrMsg(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    _ErrDoit(0, fmt, ap);
    va_end(ap);
    return;
}

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */

void
/* $f err_quit $ */
_ErrQuit(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    _ErrDoit(0, fmt, ap);
    va_end(ap);
    exit(1);
}

/* Print a message and return to caller.
 * Caller specifies "errnoflag". */

static void
_ErrDoit(int errnoflag, const char *fmt, va_list ap)
{
    int		errno_save;
    char	buf[MAXLINE];

    errno_save = errno;		/* value caller might want printed */
    vsprintf(buf, fmt, ap);
    if (errnoflag)
        sprintf(buf+strlen(buf), ": %s", strerror(errno_save));
    strcat(buf, "\n");
    fflush(stdout);		/* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(stderr);		/* SunOS 4.1.* doesn't grok NULL argument */
    return;
}

/*!
 * @brief wrap_Socket
 * @param _Family
 * @param _Type
 * @param _Protocol
 * @return
 */
inline int wrap_Socket( int _Family, int _Type, int _Protocol )
    {
    int _Sock = 0;

    if ( ( _Sock = socket( _Family, _Type, _Protocol ) ) < 0 )
        _ErrSys( "_Socket() error" );

    return _Sock;
    }

/*!
 * @brief sock_ntop
 * @param _SA
 * @param _SALen
 * @return
 */
inline char* sock_ntop( const struct sockaddr* _SA
                      , socklen_t /*_SALen*/
                      )
    {
    char _PortStr[8];
    static char _Str[128];  /* Unix domain is largest */

    switch ( _SA->sa_family )
        {
    case AF_INET:
            {
            struct sockaddr_in* _Sin = ( struct sockaddr_in* ) _SA;

            if ( inet_ntop( AF_INET, &_Sin->sin_addr, _Str, sizeof( _Str ) )
                        == nullptr )
                return nullptr;

            if ( ntohs( _Sin->sin_port ) != 0 )
                {
                std::snprintf( _PortStr, sizeof( _PortStr )
                             , ":%d", htons( _Sin->sin_port ) );
                std::strcat( _Str, _PortStr );
                }

            return _Str;
            }

    case AF_INET6:
            {
            struct sockaddr_in6* _Sin6 = ( struct sockaddr_in6* ) _SA;

            _Str[0] = '[';
            if ( inet_ntop( AF_INET6, &_Sin6->sin6_addr, _Str + 1, sizeof( _Str ) - 1 )
                        == nullptr )
                return nullptr;

            if ( ntohs( _Sin6->sin6_port ) != 0 )
                {
                std::snprintf( _PortStr, sizeof( _PortStr )
                             , "]:%d", ntohs( _Sin6->sin6_port ) );
                std::strcat( _Str, _PortStr );

                return _Str;
                }

            return _Str + 1;
            }
        }

    return nullptr;
    }

#ifdef	HAVE_SOCKADDR_DL_STRUCT
#include	<net/if_dl.h>
#endif

/*!
 * \brief sock_ntop_host
 * \param sa
 * \param salen
 * \return
 */
char* sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    static char str[128];		/* Unix domain is largest */

    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return(NULL);
        return(str);
    }

#ifdef	IPV6
    case AF_INET6: {
        struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
            return(NULL);
        return(str);
    }
#endif

#ifdef	AF_UNIX
    case AF_UNIX: {
        struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

            /* OK to have no pathname bound to the socket: happens on
               every connect() unless client calls bind() first. */
        if (unp->sun_path[0] == 0)
            strcpy(str, "(no pathname bound)");
        else
            snprintf(str, sizeof(str), "%s", unp->sun_path);
        return(str);
    }
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
    case AF_LINK: {
        struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

        if (sdl->sdl_nlen > 0)
            snprintf(str, sizeof(str), "%*s",
                     sdl->sdl_nlen, &sdl->sdl_data[0]);
        else
            snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
        return(str);
    }
#endif
    default:
        snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",
                 sa->sa_family, salen);
        return(str);
    }
    return (NULL);
}

char *
Sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    char	*ptr;

    if ( (ptr = sock_ntop_host(sa, salen)) == NULL)
        _ErrSys("sock_ntop_host error");	/* inet_ntop() sets errno */
    return(ptr);
}


/*!
 * @brief _ReadN
 * @param _Fd
 * @param _vptr
 * @param _n
 * @return
 */
inline ssize_t _ReadN( int _Fd, void* _vptr, size_t _BytesRequest )
    {
    size_t _LeftCnt = 0U;
    ssize_t _ReadCnt = 0;
    char* _ptrChar = nullptr;

    _ptrChar = ( char* )_vptr;
    _LeftCnt = _BytesRequest;
    while ( _LeftCnt > 0 )
        {
        if ( ( _ReadCnt = read( _Fd, _ptrChar, _BytesRequest ) ) < 0 )
            {
            if ( errno == EINTR )
                _ReadCnt = 0;   /* and call read() again */
            else
                return -1;
            }
        else if ( _ReadCnt == 0 )
            break;  /* EOF */

        _LeftCnt -= _ReadCnt;
        _ptrChar += _ReadCnt;
        }

    return _BytesRequest - _LeftCnt;
    }

/*!
 * @brief _WriteN
 * @param _Fd
 * @param _vptr
 * @param _n
 * @return
 */
inline ssize_t _WriteN( int _Fd, void const* _vptr, size_t _BytesRequest )
    {
    size_t _LeftCnt = 0U;
    ssize_t _WrittenCnt = 0;
    char const* _ptrChar = nullptr;

    _ptrChar = ( char* )_vptr;
    _LeftCnt = _BytesRequest;
    while ( _LeftCnt > 0 )
        {
        if ( ( _WrittenCnt = write( _Fd, _ptrChar, _BytesRequest ) ) < 0 )
            {
            if ( _WrittenCnt < 0 && errno == EINTR )
                _WrittenCnt = 0;    /* and call write() again */
            else
                return -1;      /* error */
            }

        _LeftCnt -= _WrittenCnt;
        _ptrChar += _WrittenCnt;
        }

    return _BytesRequest;
    }

int  static  _ReadCnt;
char static* _ReadPtr;
char static  _ReadBuff[ MAXLINE ];

inline ssize_t static _MyRead( int _Fd, char* _ptrChar )
    {
    if ( _ReadCnt <= 0 )
        {
    again:
        if ( ( _ReadCnt = read( _Fd
                              , _ReadBuff
                              , sizeof( _ReadBuff )
                              ) ) < 0 )
            {
            if ( errno == EINTR )
                goto again;

            return -1;
            }
        else if ( _ReadCnt == 0 )
            return 0;

        _ReadPtr = _ReadBuff;
        }

    _ReadCnt--;
    *_ptrChar = *_ReadPtr++;

    return 1;
    }

/*!
 * @brief _ReadLine
 * @param _Fd
 * @param _vptr
 * @param _MaxLen
 * @return
 */
inline ssize_t _ReadLine( int _Fd, void* _vptr, size_t _MaxLen )
    {
    ssize_t _ReqCnt = 0;
    char _Char;
    char* _ptrChar;

    _ptrChar = ( char* )_vptr;
    for ( size_t _Index = 1U; _Index < _MaxLen; _Index++ )
        {
        if ( ( _ReqCnt = _MyRead( _Fd, &_Char ) ) == 1 )
            {
            *_ptrChar++ = _Char;
            if ( _Char == '\n' )
                break;
            }
        else if ( _ReqCnt == 0 )
            {
            *_ptrChar = 0;

            return _ReqCnt - 1;
            }
        else
            return -1;
        }


    *_ptrChar = 0;

    return _ReqCnt;
    }

/*!
 * @brief _Listen
 * @param _Fd
 * @param _Backlog
 */
inline void _Listen( int _Fd, int _Backlog )
    {
    char* _ptrChar = nullptr;

    /* 4 can override 2nd argument with enviroment variable */
    if ( ( _ptrChar = getenv( "LISTENQ" ) ) != nullptr )
        _Backlog = std::atoi( _ptrChar );

    if ( listen( _Fd, _Backlog ) < 0 )
        _ErrSys( "listen() failed" );
    }

/*!
 * @brief _SockFd_to_Family
 * @param _SockFd
 * @return
 */
inline int _SockFd_to_Family( int _SockFd )
    {
    struct sockaddr_storage _SockStorage;
    socklen_t _Len = 0U;

    _Len = sizeof( _SockStorage );
    if ( getsockname( _SockFd
                    , ( struct sockaddr* ) &_SockStorage
                    , &_Len
                    ) < 0 )
        return -1;

    return _SockStorage.ss_family;
    }

typedef void _Sigfunc(int); /* for signal handlers */

/*!
 * @brief _Signal
 * @param _SigNo
 * @param _Func
 * @return
 */
inline _Sigfunc* _Signal( int _SigNo, _Sigfunc* _Func )
    {
    struct sigaction _Action;
    struct sigaction _OldAction;

    _Action.sa_handler = _Func;
    sigemptyset( &_Action.sa_mask );
    _Action.sa_flags = 0;

    if ( _SigNo == SIGALRM )
        {
#ifdef SA_INTERRUPT
        _Action.sa_flags |= SA_INTERRUPT;
#endif
        }
    else
        {
#ifdef SA_RESTRAT
        _Action.sa_flags |= SA_RESTRAT;
#endif
        }

    if ( sigaction( _SigNo, &_Action, &_OldAction ) < 0 )
        return SIG_ERR;

    return _OldAction.sa_handler;
    }

/*!
 * @brief _SigChild
 */
inline void _SigChild( int /*_SigNo*/ )
    {
    pid_t _ParentID;
    int   _Stat;

    while ( ( _ParentID = waitpid( -1, &_Stat, WNOHANG ) ) > 0 )
        std::printf( "Child %d terminated\n", _ParentID );

    return;
    }

/*!
 * \brief _HostServ
 * \param _Hostname
 * \param _Serv
 * \param _Family
 * \param _SockType
 * \return
 */
inline struct addrinfo* _HostServ( char const* _Hostname
                                 , char const* _Serv
                                 , int _Family
                                 , int _SockType
                                 )
    {
    struct addrinfo  _Hints;
    struct addrinfo* _Results;

    bzero( &_Hints, sizeof( struct addrinfo ) );
    _Hints.ai_flags = AI_CANONNAME;    /* always return canonical name */
    _Hints.ai_family = _Family;        /* AF_UNSPEC, AF_INET, AF_INET6, etc. */
    _Hints.ai_socktype = _SockType;    /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

    int _RetVal = 0;
    if ( ( _RetVal = getaddrinfo( _Hostname, _Serv, &_Hints, &_Results ) ) != 0 )
        {
        std::printf( "getaddrinfo() failed: %s", gai_strerror( _RetVal ) );

        return nullptr;
        }

    return _Results;
    }

/*!
 * \brief _TCPConnect
 * \param _Hostname
 * \param _Serv
 * \return
 */
inline int _TCPConnect( char const* _Hostname, char const* _Serv )
    {
    struct addrinfo _Hints;
    struct addrinfo *_Results;

    bzero( &_Hints, sizeof( struct addrinfo ) );
    _Hints.ai_family = AF_UNSPEC;
    _Hints.ai_socktype = SOCK_STREAM;

    int _RetVal = 0;
    if ( ( _RetVal = getaddrinfo( _Hostname, _Serv, &_Hints, &_Results ) ) != 0 )
        {
        std::fprintf( stderr, "_TCPConnect() failed for %s, %s: %s"
                    , _Hostname, _Serv, gai_strerror( _RetVal )
                    );

        exit( EXIT_FAILURE );
        }

    struct addrinfo* _ResultsSave = _Results;

    int _SockFd = 0;
     do {
        _SockFd = socket( _Results->ai_family
                        , _Results->ai_socktype
                        , _Results->ai_protocol
                        );
        if ( _SockFd < 0 )
            continue;   /* ignore this one */

        if ( connect( _SockFd, _Results->ai_addr, _Results->ai_addrlen ) == 0 )
            break;  /* success */

        close( _SockFd );
        } while ( ( _Results = _Results->ai_next ) != nullptr );

    if ( _Results == nullptr )
        _ErrSys( "_TCPConnect() failed for %s, %s", _Hostname, _Serv );

    freeaddrinfo( _ResultsSave );

    return _SockFd;
    }

int _TCPListen( char const* _Hostname
              , char const* _Serv
              , socklen_t* _ptrAddrLen
              )
    {
    struct addrinfo  _Hints;
    struct addrinfo* _Results = nullptr;
    struct addrinfo* _ResultsSave = nullptr;

    bzero( &_Hints, sizeof( _Hints ) );
    _Hints.ai_flags = AI_PASSIVE;
    _Hints.ai_family = AF_UNSPEC;
    _Hints.ai_socktype = SOCK_STREAM;

    int _RetVal = 0;
    if ( ( _RetVal = getaddrinfo( _Hostname, _Serv, &_Hints, &_Results ) ) != 0 )
        _ErrQuit( "_TCPListen() failed for %s, %s: %s"
                , _Hostname, _Serv, gai_strerror( _RetVal )
                );

    _ResultsSave = _Results;

    int _ListenFd = 0;
    int const _On = 1;
     do {
        _ListenFd = socket( _Results->ai_family
                          , _Results->ai_socktype
                          , _Results->ai_protocol
                          );
        if ( _ListenFd < 0 )
            continue;       /* error, try next one */

        setsockopt( _ListenFd, SOL_SOCKET, SO_REUSEADDR, &_On, sizeof( _On ) );
        if ( bind( _ListenFd
                 , _Results->ai_addr
                 , _Results->ai_addrlen
                 ) == 0 )
            break;          /* success */

        close( _ListenFd );
        } while ( ( _Results = _Results->ai_next ) != nullptr );

    if ( _Results == nullptr )
        _ErrSys( "_TCPListen() failed for %s, %s", _Hostname, _Serv );

    if ( listen( _ListenFd, MAXPENDING ) < 0 )
        _ErrSys( "listen() failed" );

    if ( _ptrAddrLen )
        *_ptrAddrLen = _Results->ai_addrlen;/* return size of protocol address */

    freeaddrinfo( _ResultsSave );

    return _ListenFd;
    }

#endif

 ////////////////////////////////////////////////////////////////////////////

 /***************************************************************************
 **                                                                        **
 **      _________                                      _______            **
 **     |___   ___|                                   / ______ \           **
 **         | |     _______   _______   _______      | /      |_|          **
 **         | |    ||     || ||     || ||     ||     | |    _ __           **
 **         | |    ||     || ||     || ||     ||     | |   |__  \          **
 **         | |    ||     || ||     || ||     ||     | \_ _ __| |  _       **
 **         |_|    ||_____|| ||     || ||_____||      \________/  |_|      **
 **                                           ||                           **
 **                                    ||_____||                           **
 **                                                                        **
 ***************************************************************************/
///:~
