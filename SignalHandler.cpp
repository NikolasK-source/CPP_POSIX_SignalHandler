/*
 * \file SignalHandler.cpp
 * \brief Source file de::Koesling::Signal::SignalHandler
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

#include "SignalHandler.hpp"
#include "common_header/sysexcept.hpp"
#include "common_header/destructor_exception.hpp"
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <iostream>
#include <sysexits.h>

namespace de {
namespace Koesling {
namespace Signal {

std::ostream *SignalHandler::error_stream = &std::cerr;

SignalHandler::SignalHandler(int signal_number, SignalHandler_t handler_function, int sa_flags,
        sigset_t *blocked_signals) :
        signal_number(signal_number),
        established(false)
{
    // initialize sigaction structure with 0
    memset(&curent_signal_action, 0, sizeof(curent_signal_action));

    // check handler function
    if (handler_function == nullptr) throw std::invalid_argument(
            "Unable to establish a signal handler with no handler function.");

    // SA_SIGINFO flag is set ? --> extended Signal handler must be used
    if (sa_flags & SA_SIGINFO) throw std::invalid_argument(
            "Flag SA_SIGINFO, but handler function is of the wrong type.");

    // check signal range
    if (signal_number < SIGHUP || signal_number > SIGRTMAX) throw std::invalid_argument(
            "Invalid signal number (out of range).");

    if (signal_number == SIGKILL)	// can not override KILL signal handler
    throw std::invalid_argument("Action for signal SIGKILL can not be changed.");

    if (signal_number == SIGSTOP)	// can not override STOP signal handler
    throw std::invalid_argument("Action for signal SIGSTOP can not be changed.");

    // set handler function and flags
    curent_signal_action.sa_handler = handler_function;
    curent_signal_action.sa_flags = sa_flags;

    // set signal mask if specified
    if (blocked_signals != nullptr) curent_signal_action.sa_mask = *blocked_signals;
}

SignalHandler::SignalHandler(int signal_number, SignalHandler_extended_t handler_function, int sa_flags,
        sigset_t *blocked_signals) :
        signal_number(signal_number),
        established(false)
{
    // initialize sigaction structure with 0
    memset(&curent_signal_action, 0, sizeof(curent_signal_action));

    // force flag SA_SIGINFO (because extended handler is used)
    sa_flags |= SA_SIGINFO;

    // check handler function
    if (handler_function == nullptr) throw std::invalid_argument(
            "Unable to establish a signal handler with no handler function.");

    // check signal range
    if (signal_number < SIGHUP || signal_number > SIGRTMAX) throw std::logic_error(
            "Invalid signal number (out of range).");

    if (signal_number == SIGKILL)	// can not override KILL signal handler
    throw std::logic_error("Action for signal SIGKILL can not be changed.");

    if (signal_number == SIGKILL)	// can not override STOP signal handler
    throw std::logic_error("Action for signal SIGSTOP can not be changed.");

    // set handler function and flags
    curent_signal_action.sa_sigaction = handler_function;
    curent_signal_action.sa_flags = sa_flags;

    // set signal mask if specified
    if (blocked_signals != nullptr) curent_signal_action.sa_mask = *blocked_signals;

}

SignalHandler::SignalHandler(SignalHandler &&other) noexcept :
        signal_number(std::move(other.signal_number)),
        established(std::move(other.established)),
        curent_signal_action(std::move(other.curent_signal_action)),
        old_signal_action(std::move(other.old_signal_action))
{ }

SignalHandler& SignalHandler::operator=(SignalHandler &&other) noexcept
{
    if (this != &other)	// skip self assignment
    {
        //move every element
        signal_number = std::move(other.signal_number);
        established = std::move(other.established);
        curent_signal_action = std::move(other.curent_signal_action);
        old_signal_action = std::move(other.old_signal_action);
    }
    return *this;
}

SignalHandler::~SignalHandler( )
{
    if (established)
    {
        try
        {
            revoke( );
        }
        catch (const std::exception &e) // system call failed
        {
            destructor_exception_terminate(e, *error_stream, EX_OSERR);
        }
    }
}

void SignalHandler::establish( )
{
    int temp;
    if (!established)	// initial call of establish()
    {
        // establish signal handler and store previous signal action
        temp = sigaction(signal_number, &curent_signal_action, &old_signal_action);
    }
    else	// recall of establish()
    {
        // establish signal handler, but do not override the stored
        // "old signal action"
        temp = sigaction(signal_number, &curent_signal_action, nullptr);
    }
    
    sysexcept(temp != 0, "sigaction", errno);

    established = true;
}

void SignalHandler::revoke( )
{
    if (!established)	// not established --> can not be revoked
        throw std::logic_error(
                "revoking a signal handler that has not been established will result in undefined behavior.");

    // restore old signal action
    int temp = sigaction(signal_number, &old_signal_action, nullptr);
    sysexcept(temp != 0, "sigaction", errno);

    established = false;
}

// SIG_IGN uses old style cast --> disable warning for this function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
void SignalHandler::ignore( )
{
    struct sigaction ignore_action;
    memset(&ignore_action, 0, sizeof(ignore_action));
    ignore_action.sa_handler = SIG_IGN;	// handler: ignore signal;

    int temp = sigaction(signal_number, &ignore_action, nullptr);
    sysexcept(temp != 0, "sigaction", errno);
}
// re-enable old style cast warning
#pragma GCC diagnostic pop

unsigned long SignalHandler::get_source_version( ) noexcept
{
    return SIGNAL_HANDLER_VERSION;
}

} /* namespace Signal */
} /* namespace Koesling */
} /* namespace de */
