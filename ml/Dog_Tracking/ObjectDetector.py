import cv2
import smtplib
from email.message import EmailMessage


def send_email():
    msg = EmailMessage()
    msg['Subject'] = "Motion Detected 👀"
    msg['From'] = "Dog Watchdog"
    msg['To'] = "email"
    msg.set_content("Motion has been detected")

    server = smtplib.SMTP_SSL('smtp.gmail.com', 465)
    server.login("email", "password")
    server.send_message(msg)
    server.quit()
    print('Email has been sent')


coco_names = []
coco_names_file = 'coco.names'

with open(coco_names_file, 'rt') as f:
    coco_names = f.read().rstrip('\n').split('\n')

config_path = 'ssd_mobilenet_v5_small_coco_.pbtxt'
weights_path = 'frozen_inference_graph.pb'

net = cv2.dnn_DetectionModel(weights_path, config_path)
net.setInputSize(320, 320)
net.setInputScale(1.0 / 127.5)
net.setInputMean((127.5, 127.5, 127.5))
net.setInputSwapRB(True)


def get_objects(img, draw_box=True, objects=[], threshold=0.5):
    class_ids, confs, bbox = net.detect(img, confThreshold=threshold, nmsThreshold=threshold)
    if len(objects) == 0:
        objects = coco_names

    object_info = []
    if len(class_ids) != 0:
        for classId, confidence, box in zip(class_ids.flatten(), confs.flatten(), bbox):
            class_name = coco_names[classId - 1]

            if class_name in objects:
                object_info.append([box, class_name])

                if draw_box:
                    cv2.rectangle(img, box, color=(0, 255, 0), thickness=2)
                    cv2.putText(img, class_name.upper(), (box[0] + 10, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)
                    cv2.putText(img, str(round(confidence * 100, 2)), (box[0] + 200, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)

    return img, object_info

if __name__ == '__main__':
    objs =[]
    email_sent = False

    cap = cv2.VideoCapture('Dog.mp4')
    cap.set(3, 640)
    cap.set(4, 480)

    # create video writer object
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('output.mp4', fourcc, 25.0, (640, 480))

    objs = ['dog']

    while True:
        success, img = cap.read()
        if not success:
            break

        result, object_info = get_objects(img, objects=objs, threshold=0.6)
        out.write(result)  # write frame to output video file
        cv2.imshow("Output", result)
        print(object_info)

        for i in object_info:
            if 'teddy bear' in i and not email_sent:
                send_email()
                email_sent = True

            if 'teddy bear' not in i and email_sent:
                email_sent = False

        if cv2.waitKey(40) == 27:
            break

    # release resources
    cap.release()
    out.release()
    cv2.destroyAllWindows()
