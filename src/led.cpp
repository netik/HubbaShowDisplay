#include <LedController.hpp>
#include <pins.h>
#include <safe_strlen.h>
#include <IPAddress.h>

LedController ledController(LED_DATA_PIN, LED_CLK_PIN, LED_CS_PIN, LED_NUM_SEGMENTS);

void ledShowString(const char *str)
{
  // display a short string on the LED
  int currentDigit = 0;
  for (size_t i = 0; i < safe_strlen(str, 16); i++)
  {
    if (str[i] == '\0')
    {
      break;
    }

    // if the next character is a period, set the decimal point
    ledController.setChar(0, LED_DIGITS - currentDigit, str[i], str[i + 1] == '.');

    // if the next character is a period, skip the next character
    if (str[i + 1] == '.')
    {
      i++;
    }
    currentDigit++;

    if (currentDigit > LED_DIGITS)
    {
      // we are out of space - truncate the string
      break;
    }
  }
}


void ledShowIP(IPAddress ip)
{
  // display the IP address on the LED display
  // The address is display in the format "H XXX.XXX L XXX.XXX"
  // H are the first two octets (MSB) and L shows the last two octets (LSB)
  String ipStr = ip.toString();
  const char *ipChar = ip.toString().c_str();

  // first get the length of the IP address without periods
  int ipLen = 0;
  for (size_t i = 0; i < safe_strlen(ipChar, 16); i++)
  {
    if (ipChar[i] == '.')
    {
      continue;
    }
    ipLen++;
  }

 /*
  if (ipLen <= LED_DIGITS)
  {
    // if the IP address is short enough, display it
    ledShowString(ipStr.c_str());
    return;
  }
  */

  // if the IP address is too long, display the first two octets of the
  // address and the last two octets after 1 second delay
  int firstPeriod = ipStr.indexOf('.');
  int secondPeriod = ipStr.indexOf('.', firstPeriod + 1); 
  String firstOctet = "H " + ipStr.substring(0, secondPeriod);
  String secondOctet = "L " + ipStr.substring(secondPeriod + 1, ipStr.length());

  for (int i = 0; i < 3; i++)
  {
    ledController.clearMatrix();
    ledShowString(firstOctet.c_str());
    delay(1000);

    ledController.clearMatrix();
    ledShowString(secondOctet.c_str());
    delay(1000);
  }
}

void displayDHCP()
{
  ledController.clearMatrix();
  // display ".dHCp."
  ledController.setChar(0, 6, ' ', true);
  ledController.setChar(0, 5, 'd', false);
  ledController.setChar(0, 4, 'h', false);
  ledController.setChar(0, 3, 'C', false);
  ledController.setChar(0, 2, 'p', true);
}