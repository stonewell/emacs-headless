;;; service_only_gui-win --- Summary
;;; Commentary:
;;; Code:

(declare-function x-handle-args "common-win" (args))

(cl-defmethod handle-args-function (args &context (window-system service_only_gui))
  (x-handle-args args))

(cl-defmethod frame-creation-function (params &context (window-system service_only_gui))
  (x-create-frame-with-faces params))

(provide 'service_only_gui-win)
(provide 'term/service_only_gui-win)

;;; service_only_gui-win.el ends here
