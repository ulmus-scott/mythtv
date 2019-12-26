/**
 *  DummyChannel
 */


#ifndef DUMMYCHANNEL_H
#define DUMMYCHANNEL_H

#include <QString>

#include "tv_rec.h"
#include "channelbase.h"

using namespace std;

class DummyChannel : public ChannelBase
{
  public:
    explicit DummyChannel(TVRec *parent): ChannelBase(parent)
        { (void)parent; m_curChannelName.clear(); m_curInputName.clear(); return; }
    ~DummyChannel(void) override { return; }

    bool IsTunable(const QString &/*channum*/) const override // ChannelBase
        { return true; }

    bool Open(void) override // ChannelBase
        { return InitializeInput(); }
    void Close(void) override // ChannelBase
        { return; }

    // Sets
    bool SetChannelByString(const QString &chan) override // ChannelBase
        { m_curChannelName = chan; return true; }

    // Gets
    bool    IsOpen(void)             const override // ChannelBase
        { return true; }
    QString GetDevice(void) const override // ChannelBase
        { return "/dev/dummy"; }
    QString GetCurrentInput(void)    const { return m_curInputName; }
    static uint GetCurrentSourceID(void) { return 0; }

  private:
    QString m_curInputName;
};

#endif
