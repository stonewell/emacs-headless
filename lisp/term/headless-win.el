;;; headless-win.el --- terminal set up for Headless  -*- lexical-binding:t -*-

;; Copyright (C) 2023-2024 Free Software Foundation, Inc.

;; Author: FSF
;; Keywords: terminals, i18n, headless

;; This file is part of GNU Emacs.

;; GNU Emacs is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This file contains the support for initializing the Lisp side of
;; Headless windowing.

;;; Code:

(eval-when-compile (require 'cl-lib))
(unless (featurep 'headless)
  (error "%s: Loading headless-win without having Headless"
    invocation-name))

;; Documentation-purposes only: actually loaded in loadup.el.
(require 'term/common-win)
(require 'frame)
(require 'mouse)
(require 'scroll-bar)
(require 'faces)
(require 'menu-bar)

(defvar x-invocation-args)
(defvar x-command-line-resources)

(defvar headless-initialized nil
  "Non-nil if headless windowing has been initialized.")

(add-to-list 'display-format-alist '(".*" . headless))

(declare-function headless-get-connection "headless_fns.c")
(declare-function x-handle-args "common-win")
(declare-function x-open-connection "headless_fns.c"
                  (display &optional xrm-string must-succeed))

(cl-defmethod window-system-initialization (&context (window-system headless)
                                                     &optional _ignored)
  "Set up the window system.  WINDOW-SYSTEM must be HEADLESS.
DISPLAY is ignored on Headless."
  ;; Just make sure the window system was initialized at startup.
  (cl-assert (not headless-initialized))

  ;; PENDING: not needed?
  (setq command-line-args (x-handle-args command-line-args))

  ;; Make sure we have a valid resource name.
  (when (boundp 'x-resource-name)
    (unless (stringp x-resource-name)
      (let (i)
	      (setq x-resource-name (copy-sequence invocation-name))

	      ;; Change any . or * characters in x-resource-name to hyphens,
	      ;; so as not to choke when we use it in X resource queries.
	      (while (setq i (string-match "[.*]" x-resource-name))
	        (aset x-resource-name i ?-)))))

  (x-open-connection
    "headless"
    x-command-line-resources
    ;; Exit Emacs with fatal error if this fails and we
    ;; are the initial display.
    (= (length (frame-list)) 0)
    )

  (x-apply-session-resources)

  (setq headless-initialized t)
  (message "headless window system initialize done")
)

(cl-defmethod frame-creation-function (params &context (window-system headless))
  (x-create-frame-with-faces params)
  )

(cl-defmethod handle-args-function (args &context (window-system headless))
  ;; Headless has no command line to provide arguments on.
  ;; However, call x-handle-args to handle file name args.
  (x-handle-args args))

(provide 'headless-win)
;; headless-win.el ends here.
