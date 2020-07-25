/*
 * \file SignalHandler.hpp
 * \brief Header file de::Koesling::Signal::SignalHandler
 *
 * required compiler options:
 *          -std=c++11 (or higher)
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2020 Nikolas Koesling
 *
 */

#pragma once

#include <csignal>
#include <ostream>

namespace de {
namespace Koesling {
namespace Signal {

//! signal handler function type (no SA_SIGINFO)
typedef void (*SignalHandler_t)(int);

//! signal handler function type (SA_SIGINFO)
typedef void (*SignalHandler_extended_t)(int, siginfo_t*, void*);

/* \brief Signal handler for posix signals
 *
 * Uses the posix sigaction mechanism
 */
class SignalHandler
{
    private:
        //! Signal number
        int signal_number;

        //! Indicates if the handler is established
        bool established;

        //! Signal action of this handler
        struct sigaction curent_signal_action;

        /*! \brief Previous signal action
         *
         * handler is restored by call of revoke()
         */
        struct sigaction old_signal_action;

        //! error message stream for "non-throwable" errors
        static std::ostream *error_stream;

    public:
        /*! \brief init SignalHandler
         *
         * attributes:
         *   signal_number   : signal which will be handled by this header
         *   handler_function: function void handler_funct(int signalnumber)
         *                     which is called if the signal occurs.
         *   sa_flags        : see man sigaction (sa_flags)
         *   blocked_signals : see man sigaction (sa_mask)
         * possible_throws:
         *   std::logic_error: major programming error
         */
        SignalHandler(int signal_number, SignalHandler_t handler_function, int sa_flags = 0, sigset_t *blocked_signals =
                nullptr);

        /*! \brief init SignalHandler
         *
         * attributes:
         *    signal_number   : signal which will be handled by this header
         *    handler_function: function void
         *                      handler_funct(int, siginfo_t *, void *)
         *                      which is called if the signal occurs.
         *                      (see man sigaction (sa_sigaction))
         *    sa_flags        : see man sigaction (sa_flags)
         *    blocked_signals : see man sigaction (sa_mask)
         * possible_throws:
         *    std::logic_error: major programming error
         */
        SignalHandler(int signal_number, SignalHandler_extended_t handler_function, int sa_flags = 0,
                sigset_t *blocked_signals = nullptr);

        //! delete Signal handler object
        ~SignalHandler( );

        /*! \brief arm the signal Handler
         *
         * after calling this function, the specified signal is handled by
         * this handler.
         *
         * possible_throws:
         *   std::system_error: a system call failed
         */
        void establish( );

        /*! \brief disarm the signal Handler
         *
         * after calling this function, the specified signal is handled
         * by the handler which was specified before calling
         * establish() the first time.
         *
         * possible_throws:
         *   std::system_error: a system call failed
         *   std::logic_error : major programming error
         */
        void revoke( );

        /*! \brief ignore this signal
         *
         * after calling this function, the specified signal is ignored.
         *
         * possible_throws:
         *   std::system_error: a system call failed
         */
        void ignore( );

        //! copying not allowed
        SignalHandler(const SignalHandler &other) = delete;
        //! copying not allowed
        SignalHandler& operator=(const SignalHandler &other) = delete;

        //! move this object
        SignalHandler(SignalHandler &&other) noexcept;
        //! move this object
        SignalHandler& operator=(SignalHandler &&other) noexcept;

        //! Set stream for error output for "non-throwable" errors
        inline static void set_error_stream(std::ostream &stream) noexcept;

        /*! \brief get version of header file
         *
         * only interesting if used as library.
         */
        inline static unsigned long get_header_version( ) noexcept;

        /*! \brief get version of source file
         *
         * only interesting if used as library.
         */
        static unsigned long get_source_version( ) noexcept;
};

inline void SignalHandler::set_error_stream(std::ostream &stream) noexcept
{
    error_stream = &stream;
}

} /* namespace Signal */
} /* namespace Koesling */
} /* namespace de */

#ifndef __EXCEPTIONS
static_assert(false, "Exceptions are mandatory.");
#endif
