# include "BT.h"
# include "sms.h"

int main()
{
    initPair();

    thread readDataThread(readData);
    thread writeDataThread(writeData);

    readDataThread.join();
    writeDataThread.join();

    return 0;
}