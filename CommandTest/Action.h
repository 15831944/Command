#pragma once
class CAction
{
public:     //�ܼ�
    BOOL    g_bIsStop;
public:     //�R�c���
	CAction();
	virtual ~CAction();
public:     //�B��API
    void WaitTime(HANDLE wakeEvent, int Time);
    void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
        LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
        LONG lAcceleration, LONG lInitVelociy);//���I�I��
private:    //�ۦ�B�Ψ��
    void PreventMoveError();//����b�d�X��
    void PreventGlueError();//����X���X��
    void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//�X���A�h��������ɶ�(�t�X������ϥ�)
protected:  //�����
    static DWORD WINAPI GummingTimeOutThread(LPVOID);
};


