SRC_ROOT = $(PROJECT_DIR)/../../..
INT_DIR = $(OBJROOT)
INT_CLASSES = $(INT_DIR)/Classes
OUT_DIR = $(SRC_ROOT)/../SDK/Tools/JavaTestApp

default:
# Make initial directories for intermediate files and output files, if they don't exist already
	mkdir -p $(INT_CLASSES)
	mkdir -p $(OUT_DIR)

# Generate file lists to pass to the Java compiler
	ls $(SRC_ROOT)/Source/JavaTestApp/*.java > $(INT_DIR)/JavaFiles
	ls $(SRC_ROOT)/GeneratedInterface/JavaAPI/com/microsoft/holotoolkit/sharing/*.java >> $(INT_DIR)/JavaFiles

# Compile java -> class
	javac -d $(INT_DIR)/Classes @$(INT_DIR)/JavaFiles

# Link class -> jar
	jar cvfm $(OUT_DIR)/$(PRODUCT_NAME).jar $(SRC_ROOT)/Source/JavaTestApp/Manifest.txt -C $(INT_CLASSES)/ .

clean:
	rm -rf $(INT_CLASSES)
	rm -f $(INT_DIR)/JavaFiles
	rm -f $(OUT_DIR)/$(PRODUCT_NAME).jar

fresh: clean default