  .section .rodata
  .global splash_image
  .type splash_image, @object
  .global splash_image_end
  .type splash_image_end, @object
  .align 4
splash_image:
  .incbin "binary/splash.bmp"
splash_image_end:
  .global splash_image_size
  .type splash_image_size, @object
  .align 4
splash_image_size:
  .int splash_image_end - splash_image
