# try:
from PIL import Image
import xml.etree.ElementTree as ET

border_size = 0.1
BLOCK_SIZE = 36

width  = 355
height = 355

half_width = width/float(2)
half_height = height/float(2)

BORDER_N = height * border_size
BORDER_W = width * border_size
BORDER_S = height - height * border_size
BORDER_E = width - width * border_size

xBlocks_size = width/float(BLOCK_SIZE);
yBlocks_size = height/float(BLOCK_SIZE);

def get_region(x,y):

    if x >= BORDER_W and x <= BORDER_E and y >= BORDER_N and y <= BORDER_S:
        return 'E'
    elif x <= half_width and y < half_height:
        return 'A'
    elif x <= half_width and y >= half_height:
        return 'D'
    elif x > half_width and y <= half_height:
        return 'B'
    else:
        return 'C'

def convert_histogram(histogram):

    histogram = sorted(histogram,reverse=True)

    relevant_colors_len = len(histogram) * 0.1

    block_word = ''

    for i in range(0,int(relevant_colors_len + 1)):
        block_word = block_word + str(histogram[i]) + 'x'

    return block_word

def extract_image_words(p_img):

    words = ''

    #Reduce the image to 256 colors
    im = Image.open(p_img).convert('P', palette=Image.ADAPTIVE, colors=255)

    xPoint = 0
    yPoint = 0

    for y in range(0,BLOCK_SIZE):
        for x in range(0,BLOCK_SIZE):

            his = im.transform((BLOCK_SIZE,BLOCK_SIZE), Image.EXTENT, (xPoint,yPoint,xPoint+xBlocks_size,yPoint+yBlocks_size)).histogram()
            words = words + get_region(xPoint,yPoint) + 'x' + convert_histogram(his) + ' '

            xPoint = xPoint + xBlocks_size

        xPoint = 0
        yPoint = yPoint + yBlocks_size

    return words

tree = ET.parse('p.xml')
root = tree.getroot()

chunk_index = 1
chunk_size = 5

chunk_file = None
tmp_str = ''

for produto in root.findall('produto'):

    try:
        img_name  = produto.find('img').text
        img_words = extract_image_words('colecaoDafitiPosthaus/'+img_name)

        tmp_str = tmp_str + img_name+" "+img_words+"\n"

        if chunk_index%chunk_size == 0:
            chunk_file = open("chunk_"+str(chunk_index)+".txt", "w")
            chunk_file.write(tmp_str)
            chunk_file.close()
            tmp_str = ''
            
    except Exception:
        continue;

    chunk_index = chunk_index + 1

if tmp_str != '':
    chunk_file = open("chunk_"+str(chunk_index)+".txt", "w")
    chunk_file.write(tmp_str)
    chunk_file.close()