#ifndef EMUMESSAGES_H_
#define EMUMESSAGES_H_

typedef void (*ERRCBFUNC)(char *, void *);

class EmuMessages
{
private:
	ERRCBFUNC	m_ErrorCallback;
	void*		m_ErrorClientData;

public:
	enum { DEBUG = 0, INFO, WARNING, ERROR };

	EmuMessages();
	virtual ~EmuMessages();

	void Send(int type, const char *message, ...);
	
	void setErrorCallback(ERRCBFUNC func, void *clientdata);
};

#endif /* EMUMESSAGES_H_ */
