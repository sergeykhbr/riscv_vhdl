import os, sys
from PIL import Image

class DocItem(object):
    def __init__(self, itemtype, title):
        self.itemtype = itemtype
        self.title = title
        self.subitem = []
        self.paragraph = []

    def addParagraph(self, text):
        self.paragraph.append(text)

    def save(self, doxyfile):
        if self.itemtype == "mainpage":
            doxyfile.write("@%s %s\n\n"%(self.itemtype,\
                                   self.title))
        else:
            char_to_replace = [' ', ',', ':', '.']
            itemlink = self.title
            for s in char_to_replace:
                itemlink = itemlink.replace(s, '_')
            doxyfile.write("@%s %s_%s %s\n\n"%(self.itemtype,\
                           itemlink,\
                           self.itemtype,\
                           self.title))

        for s in self.paragraph:
            doxyfile.write("  %s\n"%(s))

        for s in self.subitem:
            s.save(doxyfile)

class SubSectionItem(DocItem):
    def __init__(self, title):
        DocItem.__init__(self, "subsection", title)

class SectionItem(DocItem):
    def __init__(self, title):
        DocItem.__init__(self, "section", title)

class PageItem(DocItem):
    def __init__(self, title):
        DocItem.__init__(self, "page", title)

class MainPageItem(DocItem):
    def __init__(self, title):
        DocItem.__init__(self, "mainpage", title)
        self.current_item = None

stm32_demo =  {
                    'BTN_0':       '0_37x32x4.png',
                    'BTN_1':       '1_37x32x4.png',
                    'BTN_2':       '2_37x32x4.png',
                    'BTN_3':       '3_37x32x4.png',
                    'BTN_4':       '4_37x32x4.png',
                    'BTN_5':       '5_37x32x4.png',
                    'BTN_6':       '6_37x32x4.png',
                    'BTN_7':       '7_37x32x4.png',
                    'BTN_8':       '8_37x32x4.png',
                    'BTN_9':       '9_37x32x4.png',
                    'BTN_P1':      'p1_37x32x4.png',
                    'BTN_P2':      'p2_37x32x4.png',
                    'BTN_P3':      'p3_37x32x4.png',
                    'BTN_P4':      'p4_37x32x4.png',
                    'BTN_P5':      'p5_37x32x4.png',
                    'BTN_P6':      'p6_37x32x4.png',
                    'BTN_P7':      'p7_37x32x4.png'
                 }

"""
Doxygen proposing structure:

   title  <json_config> to doxyconfig
      page Overview: hardcoded description and help
      page N: Test name N
         section N.1: User Actions
         section N.2: Code coverage
      page N+1: Test name N+1
         section N.1: User Actions
         section N.2: Code coverage

subsection - reserved
"""
class DoxyTracer(DocItem):
    def __init__(self, sim, main_page_title="Main page"):
        DocItem.__init__(self, "mainpage", main_page_title)
        self.sim = sim
        self.screenidx = 0
        self.stepidx = 1
        self.btnPressed = []
        self.btnPressedComment = ""
        self.showBtnRelease = False
        self.curitem = self
        self.test_name = "test"
        self.addParagraph(\
"This document contains step-by-step intruction with completed test\n"\
"cases.\n"\
"TODO: generation help\n\n"
)
        self.screenshots_folder = "generated/pics/"
        if not os.path.exists(self.screenshots_folder):
            os.makedirs(self.screenshots_folder)

        # devide specific pictures. Set by setPics in device adapter
        self.loc = os.path.dirname(os.path.abspath(__file__)).replace("\\","/")
        self.stepPrefix = ""
        self.setPics(stm32_demo)

    # Method to setup button pictures locations for doxygen.
    # Get predefined dict (t34buttons, twin_buttons, ...)
    # and updates filenames to be absolute pathes
    def setPics(self, pictures_dict):
        self.pics = dict(pictures_dict)
        for k in self.pics:
            self.pics[k] = self.loc + '/stm32_demo/' + pictures_dict[k]

    def setSimulator(self, sim):
        self.sim = sim

    def resetStepIndex(self):
        self.stepidx = 1

    def addPage(self, page_name):
        self.curitem = PageItem(page_name)
        self.subitem.append(self.curitem)

    def addSection(self, section_name):
        self.curitem = SectionItem(section_name)
        self.subitem[-1].subitem.append(self.curitem)

    def addSubSection(self, subsection_name):
        self.curitem = SubSectionItem(subsection_name)
        self.subitem[-1].subitem[-1].subitem.append(self.curitem)

    def addParagraph(self, text):
        if self.curitem == self:
            DocItem.addParagraph(self, text)
        else:
            self.curitem.addParagraph(text)

    def addParagraphWithPrefix(self, text):
        self.addParagraph("{0}: {1}".format(self.genNextStepIdx(), text))

    def addPassFail(self, text):
        table_txt = r"""\begin{center}%
                     \begin{tabular}{ | m{12cm} | c |}
                     \hline
                     Observed results & Pass / Fail \\
                     \hline
                     & \makecell{ \\P / F} \\[1cm]
                     \hline
                     \end{tabular}
                     \end{center}"""

        self.addParagraph("{0}: ".format(self.genNextStepIdx()) + text)
        self.addParagraph("@latexonly {0} @endlatexonly".format(table_txt))

    def setStepPrefix(self, prefix):
        self.stepPrefix = prefix

    def genNextStepIdx(self):
        ret_val = self.stepPrefix + str(self.stepidx)
        self.stepidx += 1
        return ret_val

    def saveScreenShot(self):
        img = Image.new('RGB', (self.sim.display0.width(), self.sim.display0.height()))
        pixels = img.load()
        frame = self.sim.display0.frame
        for w in range(img.size[0]):
            for h in range(img.size[1]):
                t = frame[w*img.size[1] + h]
                pixels[w, h] = ((t >> 16) & 0xFF,
                                (t >> 8) & 0xFF,
                                (t >> 0) & 0xFF);
        name = self.test_name + "_display%d.png" % (self.screenidx)
        img.save("%s%s"%(self.screenshots_folder, name), 'PNG')

        instext = "@latexonly {\\includegraphics[scale=1.0]{../pics/%s}} @endlatexonly"\
                   %(name)
        self.addParagraph("<center>\n{0}\n</center>".format(instext))
        self.screenidx += 1


    def pressButton(self, btn, comment):
        self.btnPressed.append(btn)
        self.btnPressedComment = comment

    def releaseButton(self, btn, comment):
        self.btnPressed.remove(btn);
        #if self.showBtnRelease == True:
        #    instext = "@latexonly {\\includegraphics[scale=1.0]{%s}} @endlatexonly"\
        #            %(self.pics[btn])
        #    self.addParagraph("{0}: Release button {1} when display {2}".format(self.genNextStepIdx(), instext, comment))
        #    self.saveScreenShot()

    def go_msec_before(self, ms):
        for btn in self.btnPressed:
            instext = "@latexonly {\\includegraphics[scale=1.0]{%s}} @endlatexonly"\
                     %(self.pics[btn])
            if ms < 1000:
                self.addParagraph("{0}: Click button {1} {2}\n".format(self.genNextStepIdx(), instext, self.btnPressedComment))
            else:
                self.addParagraph("{0}: Press and hold button {1} {2}\n".format(self.genNextStepIdx(), instext, self.btnPressedComment))

    def go_msec_after(self, ms):
        if (ms < 1000):
            self.showBtnRelease = False
            return
        self.addParagraph("{0}: Wait about {1} seconds\n".format(self.genNextStepIdx(), ms / 1000))
        self.showBtnRelease = True

    def step(self, count):
        self.addParagraph("{0}: Step on {1} instructions\n".format(self.genNextStepIdx(), count))

    def set_test_name(self, name):
        self.test_name = name

    def generate(self, path):
        path = os.path.join(path, "generated", "doxygen")
        if not os.path.exists(path):
            os.makedirs(path)
#        filename = os.path.join(path, self.test_name + ".doxy")
        filename = os.path.join(path, "test.doxy")
        doxyfile = open(filename, "w")

        doxyfile.write("/**\n")
        self.save(doxyfile)
        doxyfile.write("*/\n")
        doxyfile.close()
