

void TransmitError(String msg1, byte state, String msg2, byte value) {
  BT.write(87);
  for (int ii = 0; ii < msg1.length(); ii++) {
    BT.write(msg1[ii]);
  }
  BT.write('\n');
  BT.write(state);
  for (int ii = 0; ii < msg2.length(); ii++) {
    BT.write(msg2[ii]);
  }
  BT.write('\n');
  BT.write(value);
  BT.write(200);
}