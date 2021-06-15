
void test_strings()
{
    port.getSerialString();
    delay(10);
    port.getProductString();
    delay(10);
    port.getManufacturerString();
}

void onSerialString(char *str)
{
  Serial.println(str);
}
void onProductString(char *str)
{
  Serial.println(str);
}
void onManufacturerString(char *str)
{
  Serial.println(str);
}
