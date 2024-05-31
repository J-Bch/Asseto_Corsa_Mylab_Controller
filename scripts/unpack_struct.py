import struct


# functions

def find_first_with_len_3(array):
    tmp = [x for x, y in enumerate(array) if len(y) > 2]
    return tmp[0] if len(tmp) > 0 else -1

def preprocess(structure):
    index = find_first_with_len_3(structure)

    while(index != -1):
        params = []
        for i in range(structure[index][2]):
            params.append([structure[index][0] + "_" + str(i), structure[index][1]])
            
        structure = structure[0: index] + params + structure[index + 1:]
        
        index = find_first_with_len_3(structure)
        
    return structure
    
    
def process(keys, fmt, raw):
    return dict(zip(keys, struct.unpack(fmt, raw)))

# definitions
    

_livedata_structure = preprocess([  
    ["identifier", 'h'],
    ["size", 'i'],
    ["speed_Kmh", 'f'],
    ["speed_Mph", 'f'],
    ["speed_Ms", 'f'],
    ["isAbsEnabled", '?'],
    ["isAbsInAction", '?'],
    ["isTcInAction", '?'],
    ["isTcEnabled", '?'],
    ["isInPit", '?'],
    ["isEngineLimiterOn", '?'],
    ["accG_vertical", 'f'],
    ["accG_horizontal", 'f'],
    ["accG_frontal", 'f'],
    ["lapTime", 'I'],
    ["lastLap", 'i'],
    ["bestLap", 'i'],
    ["lapCount", 'i'],
    ["gas", 'f'],
    ["brake", 'f'],
    ["clutch", 'f'],
    ["engineRPM", 'f'],
    ["steer", 'f'],
    ["gear", 'i'],
    ["cgHeight", 'f'],
    ["wheelAngularSpeed", 'f', 4],
    ["slipAngle", 'f', 4],
    ["slipAngle_ContactPatch", 'f', 4],
    ["slipRatio", 'f', 4],
    ["tyreSlip", 'f', 4],
    ["ndSlip", 'f', 4],
    ["load", 'f', 4],
    ["Dy", 'f', 4],
    ["Mz", 'f', 4],
    ["tyreDirtyLevel", 'f', 4],
    ["camberRAD", 'f', 4],
    ["tyreRadius", 'f', 4],
    ["tyreLoadedRadius", 'f', 4],
    ["suspensionHeight", 'f', 4],
    ["carPositionNormalized", 'f'],
    ["carSlope", 'f'],
    ["carCoordinates", 'f', 3]
])

live_structure_keys = [x[0] for x in _livedata_structure]
live_structure_fmt = "".join([x[1] for x in _livedata_structure])