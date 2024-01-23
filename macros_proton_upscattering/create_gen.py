import numpy as np
import matplotlib.pyplot as plt
import random
import datetime as dt

def generate_2d_3alpha():
    """
    Generates three 2D unit vectors coming out from the origin. 
    The first vector direction is chosen randomly, and the other two are 120 degrees apart from the first one.
    """
    # Random angle for the first vector
    theta1 = np.random.rand() * 2 * np.pi
    # Calculate the angles for the other two vectors
    theta2 = theta1 + (2 * np.pi / 3)  # 120 degrees apart
    theta3 = theta1 + (4 * np.pi / 3)  # 120 degrees apart
    # Generate the vectors
    v1 = np.array([np.cos(theta1), np.sin(theta1),0])
    v2 = np.array([np.cos(theta2), np.sin(theta2),0])
    v3 = np.array([np.cos(theta3), np.sin(theta3),0])
    return v1, v2, v3

def rotate_vectors_around_normal(vector, normal):
    """
    Rotates given vectors around a specified normal vector.
    """
    # Normalize the normal vector
    normal /= np.linalg.norm(normal)
    # Angle and axis for rotation
    angle = np.random.rand() * 2 * np.pi  # Random rotation angle
    axis = normal
    # Rotation matrix
    rotation_matrix = rotation_matrix_3d(axis, angle)
    # Rotated vectors
    rotated_vector = rotation_matrix.dot(vector)
    return rotated_vector

def rotation_matrix_3d(axis, theta):
    """
    Returns the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians.
    """
    axis = np.asarray(axis)
    axis = axis / np.sqrt(np.dot(axis, axis))
    a = np.cos(theta / 2.0)
    b, c, d = -axis * np.sin(theta / 2.0)
    aa, bb, cc, dd = a * a, b * b, c * c, d * d
    bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
    return np.array([[aa+bb-cc-dd, 2*(bc+ad), 2*(bd-ac)],
                     [2*(bc-ad), aa+cc-bb-dd, 2*(cd+ab)],
                     [2*(bd+ac), 2*(cd-ab), aa+dd-bb-cc]])

def generate_3d_3alpha():
    """
    Generates three 2D unit vectors and rotates them in 3D space so that they lie in a plane 
    defined by a random normal vector, originating from the origin.
    """
    # Generate 2D vectors
    v1, v2, v3 = generate_2d_3alpha()

    # Generate a random normal vector in 3D
    normal_vector = np.random.randn(3)
    normal_vector /= np.linalg.norm(normal_vector)

    # Find two orthogonal vectors in the plane perpendicular to the normal vector
    u = np.cross(normal_vector, np.array([1, 0, 0]) if normal_vector[0] != 1 else np.array([0, 1, 0]))
    u /= np.linalg.norm(u)
    v = np.cross(normal_vector, u)

    # Project the 2D vectors onto the plane
    vectors_3d = [v1[0] * u + v1[1] * v,
                  v2[0] * u + v2[1] * v,
                  v3[0] * u + v3[1] * v]

    return normal_vector, vectors_3d[0], vectors_3d[1], vectors_3d[2]


def plot_2d_vectors(v1, v2, v3):
    """
    Plots three 2D vectors on a plane.
    """
    fig, ax = plt.subplots()
    # Plot each vector
    ax.quiver(0, 0, v1[0], v1[1], angles='xy', scale_units='xy', scale=1)
    ax.quiver(0, 0, v2[0], v2[1], angles='xy', scale_units='xy', scale=1)
    ax.quiver(0, 0, v3[0], v3[1], angles='xy', scale_units='xy', scale=1)
    # Setting the aspect ratio of the plot to be equal, to maintain the scale of vectors
    ax.set_aspect('equal')
    # Setting plot limits
    plt.xlim(-1.5, 1.5)
    plt.ylim(-1.5, 1.5)
    # Adding grid, labels and title
    plt.grid(True)
    plt.xlabel('X axis')
    plt.ylabel('Y axis')
    plt.title('2D Vectors Plot')
    plt.show()


def plot_3d_vectors(v1, v2, v3):
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    # Plot each set of vectors
    # Origin
    origin = [0, 0, 0]
    # Plotting each vector
    ax.quiver(*origin, *v1, color='r', length=1.0, normalize=True)
    ax.quiver(*origin, *v2, color='g', length=1.0, normalize=True)
    ax.quiver(*origin, *v3, color='b', length=1.0, normalize=True)
    # Setting the limits and labels
    ax.set_xlim([-1, 1])
    ax.set_ylim([-1, 1])
    ax.set_zlim([-1, 1])
    ax.set_xlabel('X axis')
    ax.set_ylabel('Y axis')
    ax.set_zlabel('Z axis')
    ax.set_title('3D Plot of Unit Vectors')
    plt.show()

################################################################################################

numEvents = 100000
numTracks = 1
pdg = 2212

file_submit = open(f"submit_all.sh","w")
#for proton_energy in range(8,24+1):
count = 0
dtnow = dt.datetime.now()
for i,proton_energy in enumerate(np.arange(8.5, 17.1, 0.5)):
#for i,proton_energy in enumerate(np.arange(14.5, 17.1, 0.5)):
    gen_file_name = f"input/proton_energy{proton_energy}.gen"
    with open(gen_file_name,"w") as file1:
        print("e",file=file1)
        print(numEvents,file=file1)
        eventID = 0
        for eventID in range(numEvents):
            p1 = [0,0,proton_energy]
            vx, vy, vz = 0, 0, -500
            print(f"{eventID} {numTracks} {vx} {vy} {vz:.4f}",file=file1)
            print(f"{pdg} {p1[0]:.8f} {p1[1]:.8f} {p1[2]:.8f}",file=file1)
        num_events = eventID + 1
        print(f"energy={proton_energy} #events={num_events}")

    #for degrader_thickness in [0.05,0.1,0.15,0.2]:
    #for degrader_thickness in [0.12]:
    #for degrader_thickness in range(1,10)
    #for i in range(1,15+1):
        #degrader_thickness = i * 0.01
    #for degrader_thickness in np.arange(0.02,0.13,0.02):
    for degrader_thickness in [0.,0.02,0.04,0.06]:
        count = count+1
        print(f"[{count}] degrader_thickness={degrader_thickness:.2}")
        out_file_name = f"data/proton_sim_{dtnow.year}{dtnow.month}{dtnow.day}_e{proton_energy}_d{degrader_thickness:.2}.root"
        mac_file_name = f"input/geant4_sim_e{proton_energy}_d{degrader_thickness:.2}.mac"
        with open(mac_file_name,"w") as file1:
            content = f"""#
atomx/degraderThickness {degrader_thickness:.2} # mm

LKG4Manager/
    VisMode false
    G4InputFile {gen_file_name}
    G4OutputFile {out_file_name}
    SuppressG4InitMessage true

persistency/
    MCTrackVertex true
    MCSecondary false
    MCEdepSum false
    MCStep false

G4/
    run/setCutForAGivenParticle e- 500. mm
    run/suppressPP true
    run/beamOnAll
"""
            print(content,file=file1)
            print(f"./run_sim.exe {mac_file_name}",file=file_submit)

exit()

#numEvents = 1000
#numTracks = 3
#pdg = 1000020040
#energy = 0.125
#
#with open("input/triple_alpha.gen","w") as file1:
#    print("e",file=file1)
#    print(numEvents,file=file1)
#    for eventID in range(numEvents):
#        if eventID%100==0: print(f"event {eventID}")
#        normal, p1, p2, p3 = generate_3d_3alpha()
#        p1, p2, p3 = energy*p1, energy*p2, energy*p3
#        vx, vy, vz = 0, 0, random.uniform(0, 300)
#        print(f"{eventID} {numTracks} {vx} {vy} {vz:.4f}",file=file1)
#        print(f"{pdg} {p1[0]:.8f} {p1[1]:.8f} {p1[2]:.8f}",file=file1)
#        print(f"{pdg} {p2[0]:.8f} {p2[1]:.8f} {p2[2]:.8f}",file=file1)
#        print(f"{pdg} {p3[0]:.8f} {p3[1]:.8f} {p3[2]:.8f}",file=file1)
#
