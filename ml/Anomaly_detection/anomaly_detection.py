#!/usr/bin/env python3

# import the necessary packages
from imutils.video import VideoStream
import argparse
import datetime
import imutils
import time
import cv2

ap = argparse.ArgumentParser()
ap.add_argument("-a", "--min-area", type=int, default=500, help="minimum area size")
args = vars(ap.parse_args())

vs = cv2.VideoCapture(0)
# initialize the first frame in the video stream
referenceFrame = None
time_sent = time.time()
# loop over the frames of the video
while True:
    start=time.time()
    # grab the current frame and initialize the occupied/unoccupied
    # text
    frame = vs.read()
    frame = frame[1]
    # if the frame could not be grabbed, then we have reached the end of the video
    if frame is None:
        break
    # resize the frame, convert it to grayscale, and blur it
    frame = imutils.resize(frame, width=500)
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray_frame = cv2.GaussianBlur(gray_frame, (21, 21), 0)
    # more than 5 sec set current frame as reference
    if time.time()-time_sent > 5:
        time_sent = time.time()
        referenceFrame = gray_frame
    # if the first frame is None, initialize it
    if referenceFrame is None:
        referenceFrame = gray_frame
        continue
    # compute the absolute difference between the current frame and
    # first frame
    frameDelta = cv2.absdiff(referenceFrame, gray_frame)
    thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
    # dilate the thresholded image to fill in holes, then find contours
    # on thresholded image
    thresh = cv2.dilate(thresh, None, iterations=2)
    cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
                            cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    text = "no detection"
    # loop over the contours
    for c in cnts:
        # if the contour area is too small, ignore it
        if cv2.contourArea(c) < args["min_area"]:
            continue
        # compute the bounding box for the contour, draw it on the frame,
        # and update the text
        (x, y, w, h) = cv2.boundingRect(c)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        text = "Detected"
    # draw the text and timestamp on the frame
    cv2.putText(frame, "Status: {}".format(text), (10, 20),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
    cv2.putText(frame, datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"),
                (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
    fps=1/(time.time()-start)
    #print("Estimated frames per second : {}".format(fps))
    text = "FPS: {:.2f}".format(fps)
    cv2.putText(frame, text, (30, 40),
        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
    # show the frame and record if the user presses a key_path
    cv2.imshow("Security Feed", frame)
    cv2.imshow("Thresh", thresh)
    cv2.imshow("Frame Delta", frameDelta)
    # take key_path pressed
    key = cv2.waitKey(1) & 0xFF
    # if the `q` key_path is pressed, break from the lop
    if key == ord("q"):
        break
# cleanup the camera and close any open windows
vs.release()
cv2.destroyAllWindows()
