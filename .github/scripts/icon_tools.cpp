// SPDX-License-Identifier: GPL-3.0-or-later
// Native, standalone icon generation and source/compiled PNG validation.
// Supports Apple's CgBI (raw-deflate BGRA) as well as standard PNG.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <zlib.h>
namespace fs = std::filesystem;
using Byte = unsigned char;
struct Pixel { Byte r,g,b,a; };
struct Image { unsigned w,h; std::vector<Pixel> pixels; };
[[noreturn]] static void fail(const std::string& msg){throw std::runtime_error(msg);}
static void require(bool condition,const std::string& msg){if(!condition)fail(msg);}
static std::vector<Byte> read_file(const fs::path& file){
 std::ifstream f(file,std::ios::binary);require(bool(f),"cannot read "+file.string());
 return {std::istreambuf_iterator<char>(f),{}};
}
static void write_file(const fs::path& file,const std::vector<Byte>& data){
 if(!file.parent_path().empty())fs::create_directories(file.parent_path());
 std::ofstream f(file,std::ios::binary|std::ios::trunc);require(bool(f),"cannot create "+file.string());
 f.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
 require(bool(f),"failed writing "+file.string());
}
static uint32_t be32(const Byte* data){
 return (uint32_t(data[0])<<24)|(uint32_t(data[1])<<16)|(uint32_t(data[2])<<8)|uint32_t(data[3]);
}
static void put32(std::vector<Byte>& out,uint32_t value){
 out.push_back(static_cast<Byte>(value>>24));out.push_back(static_cast<Byte>(value>>16));
 out.push_back(static_cast<Byte>(value>>8));out.push_back(static_cast<Byte>(value));
}
static void chunk(std::vector<Byte>& output,const char (&type)[5],const std::vector<Byte>& data){
 require(data.size()<=UINT32_MAX,"PNG chunk too large");
 put32(output,static_cast<uint32_t>(data.size()));const size_t off=output.size();
 for(unsigned j=0;j<4;++j)output.push_back(static_cast<Byte>(type[j]));
 output.insert(output.end(),data.begin(),data.end());
 const uint32_t crc=crc32(0,reinterpret_cast<const Byte*>(output.data()+off),
                          static_cast<uInt>(output.size()-off));
 put32(output,crc);
}
static std::vector<Byte> unzip(const std::vector<Byte>& input,size_t expected,bool raw){
 require(expected&&expected<=64u*1024u*1024u,"PNG decoded image exceeds size limit");
 require(input.size()<=UINT32_MAX,"PNG compressed image too large");
 std::vector<Byte> result(expected);z_stream stream{};
 require(inflateInit2(&stream,raw?-MAX_WBITS:MAX_WBITS)==Z_OK,"zlib init failed");
 stream.next_in=const_cast<Byte*>(input.data());stream.avail_in=static_cast<uInt>(input.size());
 stream.next_out=result.data();stream.avail_out=static_cast<uInt>(result.size());
 const int status=inflate(&stream,Z_FINISH);
 const bool valid=status==Z_STREAM_END&&stream.total_out==expected&&stream.avail_in==0;
 inflateEnd(&stream);require(valid,"PNG decompression failed, truncated or oversized stream");return result;
}
static unsigned char paeth(int a,int b,int c){
 const int p=a+b-c,pa=std::abs(p-a),pb=std::abs(p-b),pc=std::abs(p-c);
 return static_cast<Byte>(pa<=pb&&pa<=pc?a:pb<=pc?b:c);
}
static Image parse_png(const fs::path& path){
 const auto data=read_file(path);
 constexpr std::array<Byte,8> signature{0x89,'P','N','G',13,10,26,10};
 require(data.size()>=8&&std::equal(signature.begin(),signature.end(),data.begin()),path.string()+": not a PNG");
 size_t pos=8;bool cgbi=false,ihdr=false,end=false;unsigned w=0,h=0,channels=0;
 std::vector<Byte> compressed;
 while(pos+12<=data.size()){
  const size_t size=be32(data.data()+pos);
  require(size<=data.size()-pos-12,"Malformed PNG chunk length");
  const std::string type(reinterpret_cast<const char*>(data.data()+pos+4),4);
  const Byte* payload=data.data()+pos+8;
  const auto claimed=be32(data.data()+pos+8+size);
  const auto actual=crc32(0,data.data()+pos+4,static_cast<uInt>(4+size));
  require(claimed==actual,"PNG chunk CRC mismatch");
  if(type=="CgBI")cgbi=true;
  if(type=="IHDR"){
   require(!ihdr&&size==13,"Malformed PNG IHDR");w=be32(payload);h=be32(payload+4);
   require(w>=1&&h>=1&&w<=4096&&h<=4096,"Invalid PNG dimensions");
   require(payload[8]==8&&(payload[9]==2||payload[9]==6)&&payload[10]==0&&
           payload[11]==0&&payload[12]==0,"Only 8-bit non-interlaced RGB/RGBA PNG supported");
   channels=payload[9]==2?3u:4u;ihdr=true;
  }else if(type=="IDAT"){
   require(ihdr,"IDAT before IHDR");
   require(size<=64u*1024u*1024u-compressed.size(),"Oversized IDAT");
   compressed.insert(compressed.end(),payload,payload+size);
  }else if(type=="IEND"){
   require(size==0,"Malformed IEND");end=true;break;
  }
  pos+=size+12;
 }
 require(end&&ihdr&&!compressed.empty(),"Incomplete PNG");
 const size_t stride=size_t(w)*channels;
 const auto bytes=unzip(compressed,size_t(h)*(stride+1),cgbi);
 std::vector<Byte> previous(stride,0),line(stride);Image result{w,h,{}};
 result.pixels.reserve(size_t(w)*h);size_t offset=0;
 for(unsigned y=0;y<h;++y){
  const Byte filter=bytes[offset++];require(filter<=4,"Unsupported PNG filter");
  for(size_t x=0;x<stride;++x){
   const int a=x>=channels?line[x-channels]:0,b=previous[x],c=x>=channels?previous[x-channels]:0;
   const int predictor=filter==0?0:filter==1?a:filter==2?b:filter==3?(a+b)/2:paeth(a,b,c);
   line[x]=static_cast<Byte>(bytes[offset++]+predictor);
  }
  for(size_t x=0;x<stride;x+=channels){
   Pixel p{};
   if(channels==3)p={line[x],line[x+1],line[x+2],255};
   else if(cgbi){
    p={line[x+2],line[x+1],line[x],line[x+3]};
    if(p.a&&p.a<255){
     p.r=static_cast<Byte>(std::min(255.,std::nearbyint(p.r*255./p.a)));
     p.g=static_cast<Byte>(std::min(255.,std::nearbyint(p.g*255./p.a)));
     p.b=static_cast<Byte>(std::min(255.,std::nearbyint(p.b*255./p.a)));
    }
   }else p={line[x],line[x+1],line[x+2],line[x+3]};
   result.pixels.push_back(p);
  }
  previous.swap(line);
 }
 return result;
}
using Color=std::array<Byte,3>;
constexpr int SIZE=1024;
constexpr Color BG{31,54,105},GOLD{255,190,52},WHITE{248,250,253},TEAL{28,128,145};
using Canvas=std::vector<Byte>;
static void rect(Canvas& canvas,int x0,int y0,int x1,int y1,Color color){
 x0=std::clamp(x0,0,SIZE);x1=std::clamp(x1,0,SIZE);
 y0=std::clamp(y0,0,SIZE);y1=std::clamp(y1,0,SIZE);
 for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x)
  for(int c=0;c<3;++c)canvas[(y*SIZE+x)*3+c]=color[c];
}
static void circle(Canvas& canvas,int cx,int cy,int radius,Color color){
 const int rr=radius*radius;
 for(int y=std::max(0,cy-radius);y<std::min(SIZE,cy+radius+1);++y){
  const int dy=y-cy,dx=int(std::sqrt(double(rr-dy*dy)));
  rect(canvas,cx-dx,y,cx+dx+1,y+1,color);
 }
}
static void ring(Canvas& canvas,int cx,int cy,int outer,int inner,Color color){
 const int outer2=outer*outer,inner2=inner*inner;
 for(int y=std::max(0,cy-outer);y<std::min(SIZE,cy+outer+1);++y){
  const int dy2=(y-cy)*(y-cy),ox=int(std::sqrt(double(outer2-dy2)));
  if(dy2>=inner2)rect(canvas,cx-ox,y,cx+ox+1,y+1,color);
  else{
   const int ix=int(std::sqrt(double(inner2-dy2)));
   rect(canvas,cx-ox,y,cx-ix,y+1,color);
   rect(canvas,cx+ix+1,y,cx+ox+1,y+1,color);
  }
 }
}
using Point=std::pair<int,int>;
static void polygon(Canvas& canvas,const std::vector<Point>& points,Color color){
 require(points.size()>=3,"Invalid polygon");int ymin=SIZE,ymax=0;
 for(const auto& p:points){ymin=std::min(ymin,p.second);ymax=std::max(ymax,p.second);}
 for(int y=std::max(0,ymin);y<=std::min(SIZE-1,ymax);++y){
  std::vector<double> cuts;const double scan=y+0.5;
  for(size_t j=0;j<points.size();++j){
   const auto [x1,y1]=points[j];const auto [x2,y2]=points[(j+1)%points.size()];
   if(y1==y2)continue;
   if((y1<=scan&&scan<y2)||(y2<=scan&&scan<y1))
    cuts.push_back(x1+(scan-y1)*double(x2-x1)/double(y2-y1));
  }
  std::sort(cuts.begin(),cuts.end());
  for(size_t j=0;j+1<cuts.size();j+=2)
   rect(canvas,int(cuts[j]+0.999999),y,int(cuts[j+1])+1,y+1,color);
 }
}
static void line_rect(Canvas& canvas,int x0,int y0,int x1,int y1,int thickness,Color color){
 const int dx=x1-x0,dy=y1-y0;const double len=std::sqrt(double(dx*dx+dy*dy));
 if(!len){circle(canvas,x0,y0,thickness/2,color);return;}
 const double ox=-dy/len*thickness/2,oy=dx/len*thickness/2;
 polygon(canvas,{{int(std::nearbyint(x0+ox)),int(std::nearbyint(y0+oy))},
                 {int(std::nearbyint(x1+ox)),int(std::nearbyint(y1+oy))},
                 {int(std::nearbyint(x1-ox)),int(std::nearbyint(y1-oy))},
                 {int(std::nearbyint(x0-ox)),int(std::nearbyint(y0-oy))}},color);
}
static Canvas draw_icon(){
 Canvas pixels(size_t(SIZE)*SIZE*3);
 for(size_t i=0;i<pixels.size();i+=3)std::copy(BG.begin(),BG.end(),pixels.begin()+i);
 ring(pixels,512,346,194,168,GOLD);circle(pixels,512,346,142,WHITE);
 circle(pixels,532,284,22,BG);polygon(pixels,{{586,308},{676,354},{590,390}},TEAL);
 rect(pixels,246,314,306,326,GOLD);rect(pixels,270,290,282,350,GOLD);
 rect(pixels,722,250,808,262,GOLD);rect(pixels,759,212,771,300,GOLD);
 rect(pixels,790,414,838,426,GOLD);rect(pixels,808,396,820,444,GOLD);
 circle(pixels,512,610,246,WHITE);
 polygon(pixels,{{212,610},{812,610},{812,680},{212,680}},WHITE);
 polygon(pixels,{{204,650},{472,608},{486,664},{486,840},{230,792}},WHITE);
 polygon(pixels,{{820,650},{552,608},{538,664},{538,840},{794,792}},WHITE);
 for(int y:{674,722,770}){
  line_rect(pixels,270,y,442,y-26,10,TEAL);line_rect(pixels,582,y-26,754,y,10,TEAL);
 }
 rect(pixels,501,606,523,848,GOLD);return pixels;
}
static void generate(const fs::path& output){
 const Canvas pixels=draw_icon();std::vector<Byte> raw;
 raw.reserve(size_t(SIZE)*(1+SIZE*3));
 for(int y=0;y<SIZE;++y){
  raw.push_back(0);
  raw.insert(raw.end(),pixels.begin()+size_t(y)*SIZE*3,pixels.begin()+size_t(y+1)*SIZE*3);
 }
 uLongf compressed_size=compressBound(raw.size());
 std::vector<Byte> compressed(compressed_size);
 require(compress2(compressed.data(),&compressed_size,raw.data(),raw.size(),9)==Z_OK,"icon PNG compression failed");
 compressed.resize(compressed_size);
 std::vector<Byte> png{0x89,'P','N','G',13,10,26,10},ihdr;
 put32(ihdr,SIZE);put32(ihdr,SIZE);ihdr.insert(ihdr.end(),{8,2,0,0,0});
 chunk(png,"IHDR",ihdr);chunk(png,"IDAT",compressed);chunk(png,"IEND",{});
 write_file(output,png);
 std::cout<<"generated "<<output.string()<<" ("<<png.size()<<" bytes)\n";
}
static std::array<int,3> sample(const Image& img,double nx,double ny){
 const unsigned x=std::min(img.w-1,unsigned(std::nearbyint(nx*(img.w-1))));
 const unsigned y=std::min(img.h-1,unsigned(std::nearbyint(ny*(img.h-1))));
 const Pixel p=img.pixels[size_t(y)*img.w+x];return{p.r,p.g,p.b};
}
static void validate(const fs::path& source,const fs::path& compiled,double max_error){
 const Image src=parse_png(source),dst=parse_png(compiled);
 require(src.w==1024&&src.h==1024,"source icon must be 1024x1024");
 require(dst.w>=60&&dst.h>=60&&dst.w==dst.h,"compiled icon is not square or is too small");
 constexpr std::array<double,10> coords{0.08,0.17,0.26,0.35,0.44,0.53,0.62,0.71,0.80,0.89};
 unsigned total=0;
 for(double ny:coords)for(double nx:coords){
  const auto a=sample(src,nx,ny),b=sample(dst,nx,ny);
  for(int i=0;i<3;++i)total+=unsigned(std::abs(a[i]-b[i]));
 }
 const double mean=total/300.;
 const auto white_fraction=[](const Image& image){
  size_t count=0;
  for(const Pixel& p:image.pixels)if(p.r>=245&&p.g>=245&&p.b>=245)++count;
  return double(count)/image.pixels.size();
 };
 const double src_white=white_fraction(src),dst_white=white_fraction(dst);
 std::cout<<std::fixed<<std::setprecision(2)<<"source="<<src.w<<'x'<<src.h
          <<" compiled="<<dst.w<<'x'<<dst.h<<" mean RGB error="<<mean<<'\n';
 std::cout<<std::setprecision(3)<<"near-white source="<<src_white<<" compiled="<<dst_white<<'\n';
 require(mean<=max_error,"compiled icon diverges from canonical icon");
 require(dst_white<=src_white+0.20,"compiled icon contains unexpected near-white pixels");
 std::cout<<"compiled app icon matches source within tolerance\n";
}
int main(int argc,char** argv){
 try{
  require(argc>=2,"Usage: icon-tools generate <output.png> | validate <source.png> <compiled.png> [max-mean-error]");
  const std::string command=argv[1];
  if(command=="generate"){
   require(argc==3,"Usage: icon-tools generate <output.png>");generate(argv[2]);
  }else if(command=="validate"){
   require(argc==4||argc==5,"Usage: icon-tools validate <source.png> <compiled.png> [max-mean-error]");
   double threshold=argc==5?std::stod(argv[4]):36.0;
   require(std::isfinite(threshold)&&threshold>=0,"Invalid threshold");
   validate(argv[2],argv[3],threshold);
  }else fail("Unknown icon-tools subcommand");
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"Character Profiler icon tools: "<<e.what()<<'\n';return 1;
 }
}
