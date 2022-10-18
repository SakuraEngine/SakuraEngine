import uuid
import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def main():

    resource = """{{ "guid": "{}", "type": "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74","importer": {{
            "importerType": "D5970221-1A6B-42C4-B604-DA0559E048D6",
            "configType": "b537f7b1-6d2d-44f6-b313-bcb559d3f490",
            "assetGuid": "{}"
        }},
        "cooker": {{
            "binarization": true
        }}
    }}"""
    asset = """{ "backend" : "Vulkan" }"""
    meta = """{{ "guid" : "{}" }}"""

    for i in range(0, 10000):
        fileName = "myConfig" + str(i)
        assetGuid = uuid.uuid1()
        resourceGuid = uuid.uuid1()

        write(os.path.join(BASE, "stressTest", fileName + ".meta"),
              meta.format(assetGuid))
        write(os.path.join(BASE, "stressTest", fileName + ".json"), asset)
        write(os.path.join(BASE, "stressTest", fileName + ".config.meta"),
              resource.format(resourceGuid, assetGuid))

    for i in range(10000, 15000):
        resourceGuid = uuid.uuid1()
        write(os.path.join(BASE, "stressTest", fileName + ".config.meta"),
              resource.format(resourceGuid, "b537f7b1-6d2d-44f6-b313-bcb559d3f490"))


def write(path, content):
    directory = os.path.dirname(path)
    if not os.path.exists(directory):
        os.makedirs(directory, exist_ok=True)
    open(path, "wb").write(content.encode("utf-8"))


if __name__ == "__main__":
    main()
