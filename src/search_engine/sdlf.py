from PIL import Image,ImageDraw

import xml.etree.ElementTree as ET
import operator

border_size = 0.11
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

im_new = Image.new('RGB', (355,355), (255,0,0))
dr = ImageDraw.Draw(im_new)

def draw_region(x,y):

    if x >= BORDER_W and x <= BORDER_E and y >= BORDER_N and y <= BORDER_S:
        dr.rectangle(((x,y),(x+xBlocks_size,y+yBlocks_size)), fill="black", outline = "blue")
    elif x <= half_width and y < half_height:
        dr.rectangle(((x,y),(x+xBlocks_size,y+yBlocks_size)), fill="red", outline = "blue")
    elif x <= half_width and y >= half_height:
        dr.rectangle(((x,y),(x+xBlocks_size,y+yBlocks_size)), fill="blue", outline = "blue")
    elif x > half_width and y <= half_height:
        dr.rectangle(((x,y),(x+xBlocks_size,y+yBlocks_size)), fill="yellow", outline = "blue")
    else:
        dr.rectangle(((x,y),(x+xBlocks_size,y+yBlocks_size)), fill="pink", outline = "blue")

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

    sortedHistogram = {}

    for k, v in enumerate(histogram):
        sortedHistogram[k] = v

    sortedHistogram = sorted(sortedHistogram.items(),key=operator.itemgetter(1),reverse=True)

    relevant_colors_len = int(len(sortedHistogram) * 0.95)

    block_word = ''

    j = 0
    for k,v in sortedHistogram:
        if j == relevant_colors_len:
            break

        block_word = block_word + 'x' + str(k)
        j = j + 1

    return block_word

def extract_image_words(p_img):

    words = ''

    #Reduce the image to 256 colors
    im = Image.open(p_img)
    img_reduced = im.convert('P', palette=Image.ADAPTIVE, colors=255)

    xPoint = 0
    yPoint = 0

    for y in range(0,BLOCK_SIZE):
        for x in range(0,BLOCK_SIZE):

            img_block = img_reduced.transform((BLOCK_SIZE,BLOCK_SIZE), Image.EXTENT, (xPoint,yPoint,xPoint+xBlocks_size,yPoint+yBlocks_size))
            histogram = img_block.histogram()

            words = words + get_region(xPoint,yPoint) + convert_histogram(histogram) + '\n'
            # draw_region(xPoint,yPoint)
            xPoint = xPoint + xBlocks_size

        xPoint = 0
        yPoint = yPoint + yBlocks_size

    im_new.save("rectangle.png")

    return words

print extract_image_words('consultasDafiti/1.jpg')

# tree = ET.parse('textDescDafitiPosthaus.xml')
# root = tree.getroot()

# chunk_index = 1
# chunk_size = 5000

# chunk_file = None
# tmp_str = ''

# for relevante in root.findall('relevante'):

#     try:
#         img_name  = relevante.find('img').text
#         img_words = extract_image_words('colecaoDafitiPosthaus/'+img_name)

#         tmp_str = tmp_str + img_name+" "+img_words+"\n"

#         if chunk_index%chunk_size == 0:
#             chunk_file = open("test_chunk.txt", "w")
#             chunk_file.write(tmp_str)
#             chunk_file.close()
#             tmp_str = ''
            
#     except Exception:
#         continue;

#     chunk_index = chunk_index + 1

# if tmp_str != '':
#     chunk_file = open("test_chunk.txt", "w")
#     chunk_file.write(tmp_str)
#     chunk_file.close()

# tmp_str = ''
# for i in range(1,2):
#     img_file_name = str(i)+'.jpg'
#     query_img_words = extract_image_words('consultasDafiti/'+img_file_name)
#     tmp_str = tmp_str + img_file_name+" "+query_img_words+"\n"

# img_query_file = open("test_query.txt","w")
# img_query_file.write(tmp_str)
# img_query_file.close()


#####################

# for produto in root.findall('produto'):

#     try:
#         img_name  = produto.find('img').text
#         img_words = extract_image_words('colecaoDafitiPosthaus/'+img_name)

#         tmp_str = tmp_str + img_name+" "+img_words+"\n"

#         if chunk_index%chunk_size == 0:
#             chunk_file = open("chunk_"+str(chunk_index)+".txt", "w")
#             chunk_file.write(tmp_str)
#             chunk_file.close()
#             tmp_str = ''
            
#     except Exception:
#         continue;

#     chunk_index = chunk_index + 1

# if tmp_str != '':
#     chunk_file = open("chunk_"+str(chunk_index)+".txt", "w")
#     chunk_file.write(tmp_str)
#     chunk_file.close()

# tmp_str = ''
# for i in range(1,51):
#     img_file_name = str(i)+'.jpg'
#     query_img_words = extract_image_words('consultasDafiti/'+img_file_name)
#     tmp_str = tmp_str + img_file_name+" "+query_img_words+"\n"

# img_query_file = open("img_query.txt","w")
# img_query_file.write(tmp_str)
# img_query_file.close()

