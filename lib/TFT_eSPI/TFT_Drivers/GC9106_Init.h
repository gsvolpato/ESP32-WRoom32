
// This is the command sequence that initialises the GC9106 driver
// Initial Version for 160x80 0.96 tft display

{
    Serial.println("DRIVER RUNNING");
    writecommand(0xfe);
    writecommand(0xfe);
    writecommand(0xfe);
    writecommand(0xef);

    writecommand(0xb3);
    writedata(0x03);

    writecommand(0x36);
    writedata(0xd8);

    writecommand(0x3a);
    writedata(0x05);

    writecommand(0xb6);
    writedata(0x11);
    writecommand(0xac);
    writedata(0x0b);

    writecommand(0xb4);
    writedata(0x21);

    writecommand(0xb1);
    writedata(0xc0);

    writecommand(0xe6);
    writedata(0x50);
    writedata(0x43);
    writecommand(0xe7);
    writedata(0x56);
    writedata(0x43);

    writecommand(0xF0);
    writedata(0x1f);
    writedata(0x41);
    writedata(0x1B);
    writedata(0x55);
    writedata(0x36);
    writedata(0x3d);
    writedata(0x3e);
    writedata(0x0);
    writedata(0x16);
    writedata(0x08);
    writedata(0x09);
    writedata(0x15);
    writedata(0x14);
    writedata(0xf);

    writecommand(0xF1);

    writedata(0x1f);
    writedata(0x41);
    writedata(0x1B);
    writedata(0x55);
    writedata(0x36);
    writedata(0x3d);
    writedata(0x3e);
    writedata(0x0);
    writedata(0x16);
    writedata(0x08);
    writedata(0x09);
    writedata(0x15);
    writedata(0x14);
    writedata(0xf);

    writecommand(0xfe);
    writecommand(0xff);

    writecommand(0x35);
    writedata(0x00);
    writecommand(0x44);
    writedata(0x00);
    writecommand(0x11);
//    delay_ms (120);
    writecommand(0x29);
    //writecommand(0x2c);

    writecommand(0x2A); //Set Column Address
    writedata(0x00);
    writedata(0x18);
    writedata(0x00);
    writedata(0x67);
    writecommand(0x2B); //Set Page Address
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);
    writedata(0x9f);
    writecommand(0x2c);
}
