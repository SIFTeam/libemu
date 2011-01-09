#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#include "EmuMessages.h"

static pthread_mutex_t mMutexB = PTHREAD_MUTEX_INITIALIZER;

EmuMessages::EmuMessages()
{
	this->m_ErrorCallback = NULL;
}

EmuMessages::~EmuMessages()
{

}

void EmuMessages::Send(int type, const char *message, ...)
{
#ifndef _DEBUG
	if (type <= EmuMessages::DEBUG) return;
#endif

	va_list args;
	char msg[16*1024];

	va_start (args, message);
	vsnprintf (msg, 16*1024, message, args);
	va_end (args);
	msg[(16*1024)-1] = '\0';

	pthread_mutex_lock(&mMutexB);
	if (type == EmuMessages::ERROR && this->m_ErrorCallback) this->m_ErrorCallback(msg, this->m_ErrorClientData);
	pthread_mutex_unlock(&mMutexB);

	fprintf(stdout, "%s\n", msg);
	fflush(stdout);
}

void EmuMessages::setErrorCallback(ERRCBFUNC func, void *clientdata)
{
	pthread_mutex_lock(&mMutexB);
	this->m_ErrorCallback = func;
	this->m_ErrorClientData = clientdata;
	pthread_mutex_unlock(&mMutexB);
}
