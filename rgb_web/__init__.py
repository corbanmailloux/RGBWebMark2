import requests

UPDATE_COLOR_PATH = "updateColor"
GET_COLOR_PATH = "color"

class RGBWebLight():
  def __init__(self, hostname, api_key):
    self._hostname = hostname
    self._api_key = api_key

    self._red = 0
    self._green = 0
    self._blue = 0

    if (hostname.endswith("/")):
      self._post_url = hostname + UPDATE_COLOR_PATH
      self._get_url = hostname + GET_COLOR_PATH
    else:
      self._post_url = hostname + "/" + UPDATE_COLOR_PATH
      self._get_url = hostname + "/" + GET_COLOR_PATH

  def _send_update(self, red, green, blue, transition_time):
    assert isinstance(red, int), "Value for \"red\" must be an integer"
    assert isinstance(green, int), "Value for \"green\" must be an integer"
    assert isinstance(blue, int), "Value for \"blue\" must be an integer"
    assert isinstance(transition_time, int), "Value for \"transition_time\" must be an integer"

    assert (0 <= red <= 255), "Value for \"red\" must be between 0 and 255"
    assert (0 <= green <= 255), "Value for \"green\" must be between 0 and 255"
    assert (0 <= blue <= 255), "Value for \"blue\" must be between 0 and 255"

    payload = {
      "access_token": self._api_key,
      "args": "{},{},{},{}".format(red, green, blue, transition_time)
    }
    r = requests.post(self._post_url, data=payload)
    assert r.status_code == 200, "Error connecting to light. Status code: {}".format(r.status_code)

    self._red = red
    self._green = green
    self._blue = blue

  def update_internal_status(self):
    payload = {
      "access_token": self._api_key,
    }
    r = requests.get(self._get_url, params=payload)
    assert r.status_code == 200, "Error connecting to light. Status code: {}".format(r.status_code)

    parsed_colors = r.json()["result"].split(",")
    assert len(parsed_colors) == 3

    self._red = parsed_colors[0]
    self._green = parsed_colors[1]
    self._blue = parsed_colors[2]

  def get_rgb(self):
    return [self._red, self._green, self._blue]

  def is_on(self):
    return bool(self._red or self._green or self._blue)

  def set_rgb(self, red, green, blue, transition_time = 0):
    self._send_update(red, green, blue, transition_time)

  def turn_on(self):
    self.set_rgb(255, 255, 255)

  def turn_off(self):
    self.set_rgb(0, 0, 0)
 