void BTread()
{
   byte temp=BT.read();
   if (reveive_temp1 && reveive_temp2)
   {
      if(temp!=200)
      {
        Serial.println("失去同步");     //測試要不要clear BTbuffer
        TransmitError("(ESP32)lost sync, when command: ", reveive_msg1, "value: ", temp);
        reveive_msg1=0;
        clearBT();    
      }
      else
      reveive_temp3=1;
      idle_begin_time=millis();
      reveive_temp1=0;
      reveive_temp2=0;
    Serial.println("aaaaaaa");      ////////////////////刪掉
    Serial.println(reveive_msg1);
    Serial.println(reveive_msg2);
   }
   else
   {
    if(reveive_temp1 )
      {
        reveive_temp2=1;
        reveive_msg2=temp;
      }
    else
      {
        reveive_temp1=1;
        reveive_msg1=temp;
      }
   }
      
}

void BTwrite(byte state, byte value)
{
  BT.write(state);
  BT.write(value);
  BT.write(200);
}
