/*
 * 直到接收到200 或是 BTbuffer被清光
 * 
 * 
 */
void clearBT()
{ 
  if(BT.available()>0)
  {
    while(BT.read()!=200)    //刷到直到收到200，同步為止
    {int aaa=1;              //滿足while定義，本身無意義
    Serial.println("GG");
    if(BT.available()==0)
    break;
    }
  }
}
