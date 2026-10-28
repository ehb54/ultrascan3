; UltraScan logo generator for GIMP 3.x
;
; This is the GIMP 3 port of ultrascan.scm.  The original GIMP 2 script is
; intentionally retained alongside this file.

(define (ultrascan3-fill drawable use-pattern pattern gradient reverse height)
  (if (= use-pattern TRUE)
      (begin
        (gimp-context-set-pattern pattern)
        (gimp-drawable-edit-fill drawable FILL-PATTERN))
      (begin
        (gimp-context-set-gradient gradient)
        (gimp-context-set-gradient-reverse reverse)
        (gimp-drawable-edit-gradient-fill drawable
                                          GRADIENT-LINEAR 0
                                          FALSE 1 0
                                          TRUE
                                          0 0 0 (+ height 5)))))

(define (ultrascan3-apply-overlay image outline-layer pattern)
  (let* ((width (car (gimp-drawable-get-width outline-layer)))
         (height (car (gimp-drawable-get-height outline-layer)))
         (offsets (gimp-drawable-get-offsets outline-layer))
         (overlay (car (gimp-layer-new image
                                       "Pattern overlay"
                                       width height RGBA-IMAGE
                                       100 LAYER-MODE-OVERLAY))))
    (gimp-image-insert-layer image overlay 0 -1)
    (gimp-layer-set-offsets overlay (car offsets) (cadr offsets))
    (gimp-drawable-fill overlay FILL-TRANSPARENT)
    (gimp-image-select-item image CHANNEL-OP-REPLACE outline-layer)
    (gimp-context-set-pattern pattern)
    (gimp-drawable-edit-fill overlay FILL-PATTERN)
    (gimp-selection-none image)
    ; Merge the overlay into the outline as the GIMP 2 paint-mode fill did.
    (car (gimp-image-merge-down image overlay EXPAND-AS-NECESSARY))))

(define (ultrascan3-apply-effect image
                                 logo-layer
                                 text-gradient
                                 text-gradient-reverse
                                 outline-gradient
                                 outline-gradient-reverse
                                 grow-size
                                 background-color
                                 use-pattern-text
                                 text-pattern
                                 use-pattern-outline
                                 outline-pattern
                                 use-pattern-overlay
                                 overlay-pattern
                                 shadow-toggle
                                 shadow-offset-x
                                 shadow-offset-y)
  (let* ((width (car (gimp-drawable-get-width logo-layer)))
         (height (car (gimp-drawable-get-height logo-layer)))
         (offsets (gimp-drawable-get-offsets logo-layer))
         (posx (- (car offsets)))
         (posy (- (cadr offsets)))
         (background (car (gimp-layer-new image
                                          "Background"
                                          width height RGB-IMAGE
                                          100 LAYER-MODE-NORMAL)))
         (outline-layer (car (gimp-layer-copy logo-layer))))

    (gimp-context-push)
    (gimp-context-set-defaults)

    (script-fu-util-image-resize-from-layer image logo-layer)
    (gimp-item-set-name outline-layer "Outline")
    (gimp-image-insert-layer image outline-layer 0 1)
    (gimp-item-transform-translate outline-layer posx posy)
    (gimp-image-insert-layer image background 0 2)

    (gimp-context-set-background background-color)
    (gimp-drawable-fill background FILL-BACKGROUND)

    ; Fill the face of the text.
    (gimp-image-select-item image CHANNEL-OP-REPLACE logo-layer)
    (ultrascan3-fill logo-layer
                     use-pattern-text text-pattern
                     text-gradient text-gradient-reverse height)
    (gimp-selection-none image)

    ; Grow a copy of the text alpha and fill it to make the outline.
    (gimp-image-select-item image CHANNEL-OP-REPLACE outline-layer)
    (gimp-selection-grow image grow-size)
    (ultrascan3-fill outline-layer
                     use-pattern-outline outline-pattern
                     outline-gradient outline-gradient-reverse height)
    (gimp-selection-none image)

    ; GIMP 3 exposes bump mapping as a GEGL drawable filter.
    (let* ((filter (car (gimp-drawable-filter-new outline-layer
                                                   "gegl:bump-map"
                                                   "UltraScan bump map"))))
      (gimp-drawable-filter-configure filter LAYER-MODE-REPLACE 1.0
                                      "azimuth" 110.0
                                      "elevation" 45.0
                                      "depth" 3
                                      "offset-x" 0
                                      "offset-y" 0
                                      "waterlevel" 0.0
                                      "ambient" 0.0
                                      "compensate" TRUE
                                      "invert" FALSE
                                      "type" "spherical"
                                      "tiled" FALSE)
      (gimp-drawable-filter-set-aux-input filter "aux" logo-layer)
      (gimp-drawable-merge-filter outline-layer filter))

    (gimp-layer-set-mode logo-layer LAYER-MODE-SCREEN)

    (if (= use-pattern-overlay TRUE)
        (set! outline-layer
              (ultrascan3-apply-overlay image outline-layer overlay-pattern)))

    (if (= shadow-toggle TRUE)
        (script-fu-drop-shadow image
                               (vector logo-layer)
                               shadow-offset-x shadow-offset-y
                               15 '(0 0 0) 80 TRUE))

    (gimp-selection-none image)
    (gimp-image-set-selected-layers image (vector logo-layer))
    (gimp-context-pop)))

(define (script-fu-ultrascan3-logo-alpha image
                                         drawables
                                         text-gradient
                                         text-gradient-reverse
                                         outline-gradient
                                         outline-gradient-reverse
                                         grow-size
                                         background-color
                                         use-pattern-text
                                         text-pattern
                                         use-pattern-outline
                                         outline-pattern
                                         use-pattern-overlay
                                         overlay-pattern
                                         shadow-toggle
                                         shadow-offset-x
                                         shadow-offset-y)
  (let* ((logo-layer (vector-ref drawables 0)))
    (gimp-image-undo-group-start image)
    (ultrascan3-apply-effect image logo-layer
                             text-gradient text-gradient-reverse
                             outline-gradient outline-gradient-reverse
                             grow-size background-color
                             use-pattern-text text-pattern
                             use-pattern-outline outline-pattern
                             use-pattern-overlay overlay-pattern
                             shadow-toggle shadow-offset-x shadow-offset-y)
    (gimp-image-undo-group-end image)
    (gimp-displays-flush)))

(script-fu-register-filter "script-fu-ultrascan3-logo-alpha"
  _"UltraScan 3..."
  _"Apply the UltraScan logo treatment to the selected layer's alpha"
  "Hrvoje Horvat; GIMP 3 port by the UltraScan project"
  "Hrvoje Horvat and the UltraScan project"
  "2026"
  "RGBA"
  SF-ONE-OR-MORE-DRAWABLE
  SF-GRADIENT   _"Text gradient"                         "ultrascan72"
  SF-TOGGLE     _"Reverse text gradient"                 FALSE
  SF-GRADIENT   _"Outline gradient"                      "ultrascan72"
  SF-TOGGLE     _"Reverse outline gradient"              FALSE
  SF-ADJUSTMENT _"Outline size"                          '(5 0 250 1 10 0 1)
  SF-COLOR      _"Background color"                      '(255 255 255)
  SF-TOGGLE     _"Use pattern for text"                  FALSE
  SF-PATTERN    _"Text pattern"                          "Electric Blue"
  SF-TOGGLE     _"Use pattern for outline"               FALSE
  SF-PATTERN    _"Outline pattern"                       "Electric Blue"
  SF-TOGGLE     _"Use pattern overlay"                   FALSE
  SF-PATTERN    _"Overlay pattern"                       "Parque #1"
  SF-TOGGLE     _"Shadow"                               TRUE
  SF-ADJUSTMENT _"Shadow X offset"                       '(12 0 100 1 10 0 1)
  SF-ADJUSTMENT _"Shadow Y offset"                       '(12 0 100 1 10 0 1)
)

(script-fu-menu-register "script-fu-ultrascan3-logo-alpha"
                         "<Image>/Filters/Decor")

(define (ultrascan3-create-logo text
                                size
                                font
                                text-gradient
                                text-gradient-reverse
                                outline-gradient
                                outline-gradient-reverse
                                grow-size
                                background-color
                                use-pattern-text
                                text-pattern
                                use-pattern-outline
                                outline-pattern
                                use-pattern-overlay
                                overlay-pattern
                                shadow-toggle
                                shadow-offset-x
                                shadow-offset-y)
  (let* ((image (car (gimp-image-new 256 256 RGB)))
         (text-layer (car (gimp-text-font image -1
                                          0 0 text
                                          0 TRUE size font))))
    (gimp-image-undo-disable image)
    (gimp-item-set-name text-layer text)
    (ultrascan3-apply-effect image text-layer
                             text-gradient text-gradient-reverse
                             outline-gradient outline-gradient-reverse
                             grow-size background-color
                             use-pattern-text text-pattern
                             use-pattern-outline outline-pattern
                             use-pattern-overlay overlay-pattern
                             shadow-toggle shadow-offset-x shadow-offset-y)
    (gimp-image-undo-enable image)
    image))

(define (script-fu-ultrascan3-logo text
                                   size
                                   font
                                   text-gradient
                                   text-gradient-reverse
                                   outline-gradient
                                   outline-gradient-reverse
                                   grow-size
                                   background-color
                                   use-pattern-text
                                   text-pattern
                                   use-pattern-outline
                                   outline-pattern
                                   use-pattern-overlay
                                   overlay-pattern
                                   shadow-toggle
                                   shadow-offset-x
                                   shadow-offset-y)
  (let* ((image (ultrascan3-create-logo text size font
                                        text-gradient text-gradient-reverse
                                        outline-gradient outline-gradient-reverse
                                        grow-size background-color
                                        use-pattern-text text-pattern
                                        use-pattern-outline outline-pattern
                                        use-pattern-overlay overlay-pattern
                                        shadow-toggle
                                        shadow-offset-x shadow-offset-y)))
    (gimp-display-new image)))

(script-fu-register-procedure "script-fu-ultrascan3-logo"
  _"UltraScan 3 Logo..."
  _"Create a glossy, bump-mapped UltraScan logo"
  "Hrvoje Horvat; GIMP 3 port by the UltraScan project"
  "1998-2026"
  SF-STRING     _"Text"                                  "UltraScan III"
  SF-ADJUSTMENT _"Font size (pixels)"                    '(60 2 1000 1 10 0 1)
  SF-FONT       _"Font"                                  "Times New Roman Bold Italic"
  SF-GRADIENT   _"Text gradient"                         "ultrascan72"
  SF-TOGGLE     _"Reverse text gradient"                 FALSE
  SF-GRADIENT   _"Outline gradient"                      "ultrascan72"
  SF-TOGGLE     _"Reverse outline gradient"              FALSE
  SF-ADJUSTMENT _"Outline size"                          '(2 0 250 1 10 0 1)
  SF-COLOR      _"Background color"                      '(255 255 255)
  SF-TOGGLE     _"Use pattern for text"                  FALSE
  SF-PATTERN    _"Text pattern"                          "Electric Blue"
  SF-TOGGLE     _"Use pattern for outline"               FALSE
  SF-PATTERN    _"Outline pattern"                       "Electric Blue"
  SF-TOGGLE     _"Use pattern overlay"                   FALSE
  SF-PATTERN    _"Overlay pattern"                       "Parque #1"
  SF-TOGGLE     _"Shadow"                               TRUE
  SF-ADJUSTMENT _"Shadow X offset"                       '(12 0 100 1 10 0 1)
  SF-ADJUSTMENT _"Shadow Y offset"                       '(12 0 100 1 10 0 1)
)

(script-fu-menu-register "script-fu-ultrascan3-logo"
                         "<Image>/File/Create/Logos")
