import pygame, sys
import serial # for serial port
from time import sleep

port = "COM4" #For Windows

try:
    # It seems that sometimes the port doesn't work unless 
    # you open it first with one speed, then change it to the correct value
    ser = serial.Serial(port,9600,timeout = 0.05) 
    ser.baudrate=2400
# with timeout=0, read returns immediately, even if no data
except:
    print ("Opening serial port",port,"failed")
    print ("Edit program to point to the correct port.")
    print ("Hit enter to exit")
    raw_input()
    quit()


pygame.mixer.pre_init(22050, -16, 1, 512)
#pygame.mixer.pre_init(22050, 16, 2, 4096) this was the original init, reduced buffer size to enable more responsive playback
pygame.init()
pygame.mixer.init()

screen = pygame.display.set_mode((400,300))

# load all the desired piano notes
gl = pygame.mixer.Sound('piano2\Glow.wav')
c = pygame.mixer.Sound('piano2\C.wav')  
d = pygame.mixer.Sound('piano2\D.wav')
e = pygame.mixer.Sound('piano2\E.wav')
f = pygame.mixer.Sound('piano2\F.wav')
g = pygame.mixer.Sound('piano2\G.wav')
ah = pygame.mixer.Sound('piano2\Ah.wav')

# sounds for each octave, in order of how they are mapped to fingers
oct1 = [gl, gl, gl, gl]
oct2 = [c, d, e, f]
oct3 = [g, g, ah, ah]

# initialize sounds to the first octave
sounds = oct1

# initialize our channels on which sounds can be played/stopped
channel0 = pygame.mixer.Channel(0)
channel1 = pygame.mixer.Channel(1)
channel2 = pygame.mixer.Channel(2)
channel3 = pygame.mixer.Channel(3)

channels = [channel0, channel1, channel2, channel3]

# initialize these values for our octave changing functions, octave is the current octave and changeoct1/changeoct2 are locking variables
octave = 1
changeoct1 = 0
changeoct2 = 0

sleep(10)


# play the sound at index on the channel at index, only if it is not already playing
def play_not_busy(index):
  if not channels[index].get_busy():
    channels[index].play(sounds[index])
    
# switch octaves up: change sounds and octave depending on the current octave, and set changeoct1 to 1
def higher_octave():
  global octave
  global changeoct1
  global sounds
  
  if octave == 1:
    sounds = oct2
  elif octave == 2:
    sounds = oct3
    
  octave += 1
  changeoct1 = 1
  print("up an octave") # for visual aid
  print("octave: ", octave)

# switch octaves down: change sounds and octave depending on the current octave, and set changeoct2 to 1
def lower_octave():
  global octave
  global changeoct2
  global sounds
  
  if octave == 3:
    sounds = oct2
  elif octave == 2:
    sounds = oct1
    
  octave -= 1
  changeoct2 = 1
  print("down an octave")
  print("octave: ", octave)

  
done = False

# start the main PyGame event loop
while not done:

  data = ser.read(1) # look for a character from serial port
  
  if len(data) > 0:
    data = ord(data)
    print str(data) # to see what the value of the data is
    
    # This bit is used to change to a lower octave
    # Can only change to lower octave if the current octave is greater than 1
    #and if the locking variable, changeoct2 is set to 0, which occurs when  this bit is no longer set to 1 
    if data >= 32:
      if octave > 1 and changeoct2 == 0:
        lower_octave()
      data = data % 32
    else:
      changeoct2 = 0
      
    # This bit is used to change to a higher octave
    # Can only change to higher octave if the current octave is less than 3
    #and if the locking variable, changeoct1 is set to 0, which occurs when this bit is no longer set to 1 
    if data >= 16:
      if octave < 3 and changeoct1 == 0:
        higher_octave()
      data = data % 16
    else:
      changeoct1 = 0
    
    
    # Start running through bits to play/stop notes
    # if a bit is set, we play on the corresponding channel, and move on to the next bit using the mod operation
    # if a bit is not set, then we stop playing on the corresponding channel
    if data >= 8:
      play_not_busy(3)
      data = data % 8
    else:
      channel3.stop()
      
    if data >= 4:
      play_not_busy(2)
      data = data % 4
    else:
      channel2.stop()
      
    if data >= 2:
      play_not_busy(1)
      data = data % 2
    else:
      channel1.stop()
    
    if data == 1:
      play_not_busy(0)
    else:
      channel0.stop()

  # to exit the program
  for event in pygame.event.get():
    if event.type == pygame.QUIT:
      done = True
