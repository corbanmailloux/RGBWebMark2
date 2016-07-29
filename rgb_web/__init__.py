import requests

class RGBWebLight():
  def __init__(self, hostname, api_key):
    self._hostname = hostname
    self._api_key = api_key

    self._red = 0
    self._green = 0
    self._blue = 0

  def _send_update(self, red, green, blue):
    assert isinstance(red, int), "Value for \"red\" must be an integer"
    assert isinstance(green, int), "Value for \"green\" must be an integer"
    assert isinstance(blue, int), "Value for \"blue\" must be an integer"

    assert (0 <= red <= 255), "Value for \"red\" must be between 0 and 255"
    assert (0 <= green <= 255), "Value for \"green\" must be between 0 and 255"
    assert (0 <= blue <= 255), "Value for \"blue\" must be between 0 and 255"

    payload = {
      "access_token": self._api_key,
      "args": "{},{},{}".format(red, green, blue)
    }
    r = requests.post(self._hostname, data=payload)
    assert r.status_code == 200, "Error connecting to light. Status code: {}".format(r.status_code)

    self._red = red
    self._green = green
    self._blue = blue

  def get_rgb(self):
    return [self._red, self._green, self._blue]

  def is_on(self):
    return bool(self._red or self._green or self._blue)

  def turn_on(self):
    self._send_update(255, 255, 255)

  def set_rgb(self, red, green, blue):
    self._send_update(red, green, blue)

  def turn_off(self):
    self._send_update(0, 0, 0)
 