;; set default colors
(setq my-fgcolor "#E0DFDB")
(setq my-bgcolor "#101010")
;;(set-foreground-color my-fgcolor)
;;(set-background-color my-bgcolor)

(add-to-list 'default-frame-alist '(foreground-color . "#E0DFDB"))
(add-to-list 'default-frame-alist '(background-color . "#101010"))

;;
;; open scratch file
(find-file "C:/Users/vswam/GoogleDrivePersonal/scratch.txt")
	   
;;
;; hide splash screen
(setq inhibit-startup-screen t)

;;
;; basic editing setup
(setq c-basic-offset 3) ; indent 3 chars
(setq tab-width 3)      ; tab also 3 chars

;;
;; I like my Ctrl-C, Ctrl-V, Ctrl-A
(cua-mode t)
(setq cua-auto-tabify-rectangles nil) ;; Don't tabify after rectangle commands
(transient-mark-mode 1) ;; No region when it is not highlighted
(setq cua-keep-region-after-copy t) ;; Standard Windows behaviour(cua-mode t)

;;
;; turn off irritating sound
(setq ring-bell-function 'ignore)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(cua-mode t nil (cua-base)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:family "Consolas" :foundry "outline" :slant normal :weight normal :height 98 :width normal)))))

