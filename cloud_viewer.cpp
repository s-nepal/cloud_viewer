#include <pcl/visualization/cloud_viewer.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <iostream>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/passthrough.h>

#include <boost/thread/thread.hpp>
#include <pcl/common/common_headers.h>
#include <pcl/range_image/range_image.h>
#include <pcl/visualization/range_image_visualizer.h>


#include <string>
#include <fstream>
#include <vector>

#include "data.cpp" // import data from .pcap file

using namespace std;

int user_data;
const int packet_size = 1248;

struct fire_data {
	uint16_t block_id;
	double azimuth;
	double dist[32];
	double intensity[32];
};

struct data_packet {
	uint8_t header[42];
	fire_data payload[12];
	uint8_t footer[6];
};


void pp_callback(const pcl::visualization::PointPickingEvent& event, void* viewer_void)
{
   cout << "Hello World" << endl;
   std::cout << "Picking event active" << std::endl;
   if(event.getPointIndex()!=-1)
   {
       float x,y,z;
       event.getPoint(x,y,z);
       std::cout << x << ";" << y <<";" << z << std::endl;
   }
}

void 
viewerOneOff (pcl::visualization::PCLVisualizer& viewer)
{	

    viewer.setBackgroundColor (0,0,0); // black background
	viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE,2,"cloud"); // size of point clouds
	viewer.setRepresentationToSurfaceForAllActors();
	viewer.addCoordinateSystem (4);
	viewer.initCameraParameters ();
	viewer.setCameraPosition (-20, 0, 60, 0.0, 0, 100);
	

	// add labels for the 3 axes
	/*pcl::PointXYZ pos;
	pos.x = 20; pos.y = 0; pos.z = 0;
	viewer.addText3D("x",pos,2,1.0,0.0,0.0,"x");
	pos.x = 0; pos.y = 20; pos.z = 0;
	viewer.addText3D("y",pos,2,0.0,7,0.0,"y");
	pos.x = 0; pos.y = 0; pos.z = 20;
	viewer.addText3D("z",pos,2,0.0,0.0,20,"z");*/
	
    pcl::PointXYZ o;
    o.x = 0;
    o.y = 0;
    o.z = 0;

   /* viewer.addSphere (o, 15, "sphere", 0);
	viewer.addCube(-1,1,0,1,-1,1,1.0,1.0,1.0,"cube",0);
    std::cout << "i only run once" << std::endl;*/
    
}


void 
viewerPsycho (pcl::visualization::PCLVisualizer& viewer)
{
    static unsigned count = 0;
    std::stringstream ss;
    ss << "Once per viewer loop: " << count++;
    viewer.removeShape ("text", 0);
    //viewer.addText (ss.str(), 200, 300, "text", 0);

    //FIXME: possible race condition here:
    user_data++;	
}

void delay()
{
	for(int i = 0; i < 30000; i++){
		for (int j = 0; j < 10000; j++){}
	}
}
    
int main (int argc, char **argv)
{
	const clock_t begin_time = clock();

	struct data_packet first; // define the data packet

	for(int i = 0; i < 42; i++) // populate header data that is 42 bytes long
		first.header[i] = pkt1[i];

	for(int i = 0; i < 6; i++) // populate footer data that is 6 bytes long
		first.footer[i] = pkt1[i + 1242];

	// populate the payload (block ID, azimuth, 32 distances, 32 intensities  for each of the 12 data blocks)
	int curr_byte_index = 42; // not 43 bcz. in C++, indexing starts at 0, not 1
	uint8_t curr_firing_data[100];
	fire_data temp[12];

	for(int i = 0; i < 12; i++){
		for(int j = 0; j < 100; j++){
			curr_firing_data[j] = pkt1[j + curr_byte_index];
			//cout << (double)curr_firing_data[j] << endl;
		}
		temp[i].block_id = (curr_firing_data[1] << 8) | (curr_firing_data[0]);
		temp[i].azimuth = (double)((curr_firing_data[3] << 8) | (curr_firing_data[2])) / 100;

		/*cout << temp[0].block_id << " " << temp[0].azimuth << endl;*/

		int ctr = 0;
		for(int j = 0; j < 32; j++){
			temp[i].dist[j] = (double)((curr_firing_data[4 + ctr + 1] << 8) | curr_firing_data[4 + ctr]) / 500;
			temp[i].intensity[j] = curr_firing_data[4 + ctr + 2];
			ctr = ctr + 3;
		}
		first.payload[i] = temp[i];
		curr_byte_index = curr_byte_index + 100;
	}

	/*for (int i = 0; i < 32; i++)
		cout << (double)first.payload[11].dist[i] << endl;
*/

	
	// declare the point cloud class
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZRGBA>);	
	
	pcl::PointXYZRGBA sample;

	pcl::visualization::CloudViewer viewer("Cloud Viewer");

    //blocks until the cloud is actually rendered
    //viewer.showCloud(cloud);
	viewer.runOnVisualizationThreadOnce (viewerOneOff);
    viewer.runOnVisualizationThread (viewerPsycho);
  	
	while (!viewer.wasStopped()){
	//for(int i = 0; i < 1000000; i++){
		for(int j = 0; j < 1000; j++){

			int color = rand()%3;
			sample.x = rand()%100;
			sample.y = rand()%100;
			sample.z = rand()%100;

			if(color == 0){
				sample.r = 255; sample.g = 0; sample.b = 0;
			} else if(color == 1) {
				sample.r = 0; sample.g = 255; sample.b = 0;
			} else {
				sample.r = 0; sample.g = 0; sample.b = 255;
			}

			cloud -> points.push_back(sample);
		}
		viewer.showCloud(cloud);
		cloud -> points.clear();
		delay();
	}

    return 0;
}