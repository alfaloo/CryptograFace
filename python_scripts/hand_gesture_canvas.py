import cv2 as cv
import mediapipe as mp
import numpy as np
from enum import Enum

class Finger(Enum):
    THUMB = 1
    INDEX = 2
    MIDDLE = 3
    RING = 4
    PINKY = 5

def is_extended(finger, landmarks):
    # Thumb: Landmarks 1 to 4
    # Index Finger: Landmarks 5 to 8
    # Middle Finger: Landmarks 9 to 12
    # Ring Finger: Landmarks 13 to 16
    # Pinky Finger: Landmarks 17 to 20

    tip_y = landmarks[finger.value * 4][2]
    joint_y = landmarks[finger.value * 4 - 2][2]
    palm_y = landmarks[0][2]

    # Check if the fingertip is above the PIP joint
    if (tip_y < joint_y and tip_y > palm_y) or (tip_y > joint_y and tip_y < palm_y):
        return False
    else:
        return True

def main(rawCanvas):
    # Initialize webcam video capture
    cap = cv.VideoCapture(0)

    # Initialize mediapipe Hands object for hand tracking
    mpHands = mp.solutions.hands
    hands = mpHands.Hands()
    mpdraw = mp.solutions.drawing_utils

    col = [0, 0, 255]  # Default color (red)
    thickness = 25

    prev_x, prev_y = 0, 0

    # Create a blank canvas to draw on
    if len(rawCanvas) == 0:
        canvas = np.zeros((1080, 1920, 3), dtype=np.uint8)
    else:
        canvas = np.frombuffer(rawCanvas, dtype=np.uint8).reshape((1080, 1920, 3))

    color_options = [(0, 0, 0), (0, 0, 255), (0, 127, 255), (0, 255, 255), (0, 255, 0), (255, 0, 0), (130, 0, 75), (211, 0, 148)]

    while True:
        # Read a frame from the webcam and flip it horizontally
        success, frame = cap.read()
        frame = cv.flip(frame, 1)

        frame_height, frame_width, _ = frame.shape

        # Draw color selection rectangles
        rect_height = int(frame.shape[0] / 10)
        rect_width = int(frame.shape[1] / 8)
        color_rects = [(i * rect_width, 0, rect_width, rect_height, color) for i, color in enumerate(color_options)]

        for rect in color_rects:
            x, y, width, height, color = rect
            cv.rectangle(frame, (x, y), (x + width, y + height), color, -1)  # -1 fills the rectangle

        # Define the points of the triangle
        pt1 = (frame_width, frame_height)
        pt2 = (frame_width - rect_height, rect_height)
        pt3 = (frame_width, rect_height)

        # Points must be in a numpy array shaped as ROWSx1x2 for fillPoly
        triangle_slider = np.array([pt1, pt2, pt3])

        cv.fillPoly(frame, [triangle_slider], (255, 255, 255))

        # Convert the frame to RGB color space for hand tracking
        img = cv.cvtColor(frame, cv.COLOR_BGR2RGB)

        # Process the frame to detect hand landmarks
        results = hands.process(img)
        landmarks = []

        if results.multi_hand_landmarks:
            for hn in results.multi_hand_landmarks:
                for id, lm in enumerate(hn.landmark):
                    h, w, c = frame.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    landmarks.append([id, cx, cy])
                mpdraw.draw_landmarks(frame, hn, mpHands.HAND_CONNECTIONS)

        if len(landmarks) != 0:

            # Check if the hand is in "selection mode" or "drawing mode"
            index_x, index_y = landmarks[8][1], landmarks[8][2]

            # Detect the color chosen by the hand position
            if index_y < 100:

                # Reset previous position
                prev_x, prev_y = 0, 0

                col = color_options[int(index_x / rect_width)]

            elif index_x > (frame_width - rect_height):

                # Reset previous position
                prev_x, prev_y = 0, 0

                thickness = int((frame_height - index_y) / (frame_height - rect_height) * 90 + 10)

            elif is_extended(Finger.INDEX, landmarks) and not is_extended(Finger.MIDDLE, landmarks)  and not is_extended(Finger.RING, landmarks)  and not is_extended(Finger.PINKY, landmarks):
                if prev_x == 0 and prev_y == 0:
                    prev_x, prev_y = index_x, index_y

                # Draw lines on the canvas when in "drawing mode"
                if col == (0, 0, 0):
                    cv.line(frame, (prev_x, prev_y), (index_x, index_y), col, 2 * thickness, cv.FILLED)
                    cv.line(canvas, (prev_x, prev_y), (index_x, index_y), col, 2 * thickness, cv.FILLED)
                else:
                    cv.line(frame, (prev_x, prev_y), (index_x, index_y), col, thickness, cv.FILLED)
                    cv.line(canvas, (prev_x, prev_y), (index_x, index_y), col, thickness, cv.FILLED)

                prev_x, prev_y = index_x, index_y

            else:
                # Reset previous position
                prev_x, prev_y = 0, 0

            cv.circle(frame, (index_x, index_y), thickness, col, 2)

        # Prepare the canvas for blending with the frame
        imgGray = cv.cvtColor(canvas, cv.COLOR_BGR2GRAY)
        _, imgInv = cv.threshold(imgGray, 50, 255, cv.THRESH_BINARY_INV)
        imgInv = cv.cvtColor(imgInv, cv.COLOR_GRAY2BGR)

        # Use bitwise operations to blend the frame with the canvas
        imgInv = cv.resize(imgInv, (frame.shape[1], frame.shape[0]))
        canvas = cv.resize(canvas, (frame.shape[1], frame.shape[0]))
        frame = cv.bitwise_and(frame, imgInv)
        frame = cv.bitwise_or(frame, canvas)

        # Show the webcam frame and the canvas
        cv.imshow('cam', frame)

        # Break the loop if the 'q' key is pressed
        if cv.waitKey(1) & 0xFF == ord('q'):
            cap.release()
            cv.destroyAllWindows()
            break

    return canvas.tostring()