#!/usr/bin/env python


f = open("font.bdf", "r")
data = f.read().split("\n")
f.close()

charmap = {}

def MakeChar(lines):
  charName = lines[0][10:]
  encoding = -1
  fontBytes = ""

  readingbytes = False

  for line in lines:
    if readingbytes:
      if "ENDCHAR" in line:
        readingbytes = False
      else:
        fontBytes += chr(int(line, 16))
    else:
      if "ENCODING" in line:
        encoding = int(line[9:])
      elif "BITMAP" in line:
        readingbytes = True

  print "Making Char %s" % charName
  return {
    "charName": charName,
    "encoding": encoding,
    "fontBytes": fontBytes
  }


charlinebuff = []
readingchardata = False

for line in data:
  if not readingchardata:
    # Search for STARTCHAR
    if "STARTCHAR" in line:
      readingchardata = True

  if readingchardata:
    charlinebuff.append(line)

    if line == "ENDCHAR":
      readingchardata = False
      c = MakeChar(charlinebuff)
      charmap[c["encoding"]] = c
      charlinebuff = []

# Build 256 char table

totalChars = 256
charsPerLine = 16
charHeight = 10

table = bytearray("\x00" * totalChars * charHeight)

for i in charmap:
  if i < totalChars:
    c = charmap[i]
    # print c
    x = (i & 0xF)
    y = ((i & 0xFFFFF0) >> 4) * charHeight
    for z in range(len(c["fontBytes"])):
      table[y * charsPerLine + z * charsPerLine + x] = c["fontBytes"][z]

f = open("font.bin", "wb")
f.write(table)
f.close()

f = open("font.h", "w")
f.write("// Total Chars: %d\n" % totalChars)
f.write("// Char Width: 6 pixels\n")
f.write("// Char Line Byte Width: 1 byte\n")
f.write("// Char Number of lines: %d\n" %charHeight)
f.write("// Location Formula: \n")
f.write("// For \n")
f.write("//   D = ASCII Code\n")
f.write("//   n = Line Number\n")
f.write("//   int x = (D & 0xF)\n")
f.write("//   int y = ((D & 0xF0) >> 4) * %d\n" %charHeight)
f.write("//   font%d_%d[y * %d + n * %d + x]\n" %(charsPerLine, charHeight,charsPerLine, charsPerLine))
f.write("\n")
f.write("const uint8_t font%d_%d_height = %d;\n" % (charsPerLine, charHeight, charHeight))
f.write("const uint8_t font%d_%d_charsPerLine = %d;\n" %(charsPerLine, charHeight,charsPerLine))
f.write("const uint8_t font%d_%d_charWidth = 6;\n" %(charsPerLine, charHeight))
f.write("const uint8_t font%d_%d[%d] = {" % (charsPerLine, charHeight, totalChars * charHeight))

for i in range(len(table)):
  if i % 16 == 0:
    f.write("\n    ")
  f.write("0x%02x" % table[i])
  if i != len(table) - 1:
    f.write(", ")

f.write("\n};\n")
f.close()


# Bank 2
table = bytearray("\x00" * totalChars * charHeight)

for i in charmap:
  if i - 0x2500 < totalChars and i > 0x2500 - 1:
    c = charmap[i]
    print c
    i -= 0x2500
    x = (i & 0xF)
    y = ((i & 0xFFFFF0) >> 4) * charHeight
    for z in range(len(c["fontBytes"])):
      table[y * charsPerLine + z * charsPerLine + x] = c["fontBytes"][z]

f = open("font2.bin", "wb")
f.write(table)
f.close()


f = open("font2.h", "w")
f.write("// Total Chars: %d\n" % totalChars)
f.write("// Char Width: 6 pixels\n")
f.write("// Char Line Byte Width: 1 byte\n")
f.write("// Char Number of lines: %d\n" %charHeight)
f.write("// Location Formula: \n")
f.write("// For \n")
f.write("//   D = ASCII Code\n")
f.write("//   n = Line Number\n")
f.write("//   int x = (D & 0xF)\n")
f.write("//   int y = ((D & 0xF0) >> 4) * %d\n" %charHeight)
f.write("//   font%d_%d[y * %d + n * %d + x]\n" %(charsPerLine, charHeight,charsPerLine, charsPerLine))
f.write("\n")
f.write("const uint8_t bank2_font%d_%d_height = %d;\n" % (charsPerLine, charHeight, charHeight))
f.write("const uint8_t bank2_font%d_%d_charsPerLine = %d;\n" %(charsPerLine, charHeight,charsPerLine))
f.write("const uint8_t bank2_font%d_%d_charWidth = 6;\n" %(charsPerLine, charHeight))
f.write("const uint8_t bank2_font%d_%d[%d] = {" % (charsPerLine, charHeight, totalChars * charHeight))

for i in range(len(table)):
  if i % 16 == 0:
    f.write("\n    ")
  f.write("0x%02x" % table[i])
  if i != len(table) - 1:
    f.write(", ")

f.write("\n};\n")
f.close()

