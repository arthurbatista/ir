import sys
import math
import operator
import xml.etree.ElementTree as ET

from PIL import Image

PATH_PRODUCTS_DESC = sys.argv[1]
FOLDER_COLECAO_IMG = sys.argv[2]
FOLDER_CONSULTA    = sys.argv[3]

border_size = 0.11

NUMBER_OF_BLOCKS = 1296

BLOCK_SIZE = 36
    
def get_region(width, height, x, y):
    imgWBorder = width * border_size
    imgHBorder = height * border_size

    if (x <= (width/2) and y <= imgHBorder) or (x <= imgWBorder and y <= height/2):
        return "A"
    if (x > (width/2) and y <= imgHBorder) or (x > (width - imgWBorder) and y <= height/2):
        return "B"
    if (x <= (width/2) and y > (height - imgHBorder)) or (x <= imgWBorder and y > height/2):
        return "D"
    if (x > (width/2) and y > (height - imgHBorder)) or (x > (width - imgWBorder) and y > height/2):
        return "E"
    return "C"

def extract_image_words(imagePath):

    im = Image.open(imagePath)

    #quantize 256
    img256 = im.convert('P')

    imgWidth = im.size[0]
    imgHeight = im.size[1]

    blockSize = math.sqrt((imgWidth * imgHeight) / NUMBER_OF_BLOCKS)

    yleft = 0
    yright = blockSize

    img_vocab = ''

    for i in range(0, 36):
        xleft = 0
        xright = blockSize

        for j in range(0, 36):
            block = img256.transform((36, 36), Image.EXTENT, (xleft, yleft, xright, yright))
            block_histogram = block.histogram()

            histogram = {}

            for key, value in enumerate(block_histogram):
                histogram[key] = value

            sorted_histogram = sorted(histogram.items(), key = operator.itemgetter(1), reverse=True)

            five_percent = int(len(sorted_histogram) * 0.95)

            blockWord = get_region(imgWidth, imgHeight, xright, yright)

            if blockWord == 'C':
                _bin = 0

                for value in sorted_histogram:

                    #discard five percent of less frequent colors
                    if _bin == five_percent:
                        break

                    blockWord = blockWord + 'x' + str(value[0])

                    _bin = _bin + 1

                img_vocab = img_vocab + blockWord + ' '

            xleft = xleft + blockSize
            xright = xright + blockSize

        yleft = yleft + blockSize
        yright = yright + blockSize

    return img_vocab


tree = ET.parse(PATH_PRODUCTS_DESC)
root = tree.getroot()

chunk_index = 1
chunk_filename_index = 1
chunk_size = 3000

chunk_file = None
tmp_str = ''

for produto in root.findall('produto'):

    try:
        img_name  = produto.find('img').text
        img_words = extract_image_words(FOLDER_COLECAO_IMG+img_name)
        tmp_str = tmp_str + img_name+" "+img_words+"\n"

        if chunk_index%chunk_size == 0:
            chunk_file = open("sdlf/chunk_"+str(chunk_filename_index)+".txt", "w")
            chunk_file.write(tmp_str)
            chunk_file.close()
            tmp_str = ''
            chunk_filename_index = chunk_filename_index + 1
    except Exception:
        continue;

    chunk_index = chunk_index + 1

if tmp_str != '':
    chunk_file = open("sdlf/chunk_"+str(chunk_filename_index)+".txt", "w")
    chunk_file.write(tmp_str)
    chunk_file.close()

tmp_str = ''
for i in range(1,51):
    img_file_name = str(i)+'.jpg'
    query_img_words = extract_image_words(FOLDER_CONSULTA+img_file_name)
    tmp_str = tmp_str + query_img_words + "\n"

img_query_file = open("sdlf/queries.txt","w")
img_query_file.write(tmp_str)
img_query_file.close()