# lv_zifont
Interface to use a .zi V5 font at run time

## .zi V5 font files
Obtain or create a font file:
- Download the HMI Font Pack from [here](https://sourceforge.net/projects/freetype/files/)
- Use Nextion Editor or USART Editor to generate a .zi font file

## Add lv_zifont to your project
- Add library: `lv_lib_zifont`
- Include `lv_zifont.h` in your project

## Usage in LittlevGL
```c
    lv_zifont_init();

    static lv_font_t font1;
    lv_zifont_font_init(&font1, "./notosans_32.zi", 0);

    static  lv_style_t ft_style;
    lv_style_copy(&ft_style,  &lv_style_plain);

    ft_style.text.font = &font1;
    lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &ft_style);
    lv_label_set_text(label, "Hello word");
```

## Learn more
- ZI font format version 6 [specification](https://github.com/hagronnestad/nextion-font-editor/blob/master/Docs/Nextion%20Font%20Format%20Specification%20ZI%20version%206.md) 
- LittlevGL's [font interface](https://docs.littlevgl.com/en/html/overview/font.html#add-a-new-font-engine)
