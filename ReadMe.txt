1 ��Ŀ���
 Gs9330��Ŀ�ı�����ģ�顣

2 ����Ŀ¼�ṹ
 2.1�����д��붼������CodesĿ¼�¡�
 2.2��Codes/Include ���ڴ�Ź���ģ����Ҫ������ģ�����õ�.h�ļ�����Ŀ¼���д��붼����Gs9330�����ռ䡣
 2.4��Codes/Src ���ڴ�Ź���ģ��.cpp�ļ�����Ŀ¼���д��붼����Gs9330�����ռ䡣

3 �ύ����
 Ϊ�˱����˷ѹ����ʱ�����Ŀ��������Ա���۴����Project�Ĳ�νṹ������Ŀ¼/�ļ����ύ����Ŀ�ĵ�svn server:
 3.1) Gs9330.sln
 3.2) VcProject, VcUnitTestProject
 3.3) Documents

ver.2.2:  �������µĹ������, �����޸���������֪��bug.
ver.2.5:  ֧�ֽ�����table��Ϊ���section�Ĺ���, ���ʱ nit �� bat �����ts���벻ͬ��section��. sdt �����service��ϸ��Ϣ���벻ͬ��section��. 
          eit�����event��Ϣ���벻ͬ��section��.
ver.2.6:  ��ʽ��ʼ�ύ���Բ�����, �ӱ��汾��ʼ, ֮�����д����bug��ͳ��ΪCoding Bug.
          �����һЩ���õ�unit test�Ĵ���.  ������unit test�Ľ���Դ�������һЩ�޸�.
ver.2.7:  Ϊ���������Ż�, �صش���һ��branch,  ���������Ż��Ĵ���Ӱ���� ver.2.6 �ȶ��Ĵ���.
          ����iso13818-1�������� �޸����� TsPacketInterface => TransportPacketInterface, TsPacketsInterface => TransportPacketsInterface.
ver.2.8:  �Զ�ȡxml�ļ���Ч�������Ż�, �Ż����ȡ32��ts һ�ܵ�eit��Ϣֻ��Ҫ3~4��(�Ż�ǰ��Լ��Ҫ50��).
          ������һЩ��Ԫ��������.
          ��ver.2.8�Ļ�����, ������һ��ver.Debug.2.8. ���ڲ��Զ��߳��Ƿ�Զ�ȡxml�ļ���Ч�������Ե������� Ч�������롣
          Ver.Debug.2.8 �����ڽ������������ܲ��ԵĻ����汾��
ver.2.9:  ���Ӷ�����Ƶ���֧��, receiver.xmlÿ������ĵ�һ��receiver ��������Ƶ��.
          ����ģ���� SiTableTemplate ��Ϊsdt, bat, nit, eit�Ļ���, ����SiTableTemplate, 4�ű������GetCodeSize(), GetSecNumber()��������ܶ�����Ĵ���.
ver.3.0:  ͨ������������ȡ�������õ�·��.
          �޸�SiTableXmlWrapperInterface::Select()������, �µĺ�������ͨ��return ����xml�ж�ȡ����SiTable��
          ������"�躣"���ȶ��汾.
ver.3.1:  �޸����躣����ʱ���ֵ������ϵ�bug. 
          1 ��������, ֧���鲥��ַ  ��ΪEpgSender��Ŀ�ĵ�ַ. 
          2 ��������, ֧�ָ��ݲ�ͬ��tsid,��eit��pidӳ��Ϊ��ͬ��pid.
          
4 ʵ������
    983,116�ֽڵ�xml������TsΪ178,788�ֽ�.
    
5 ��ע
    ��ʵ�������,ż���ᷢ��EpgSender.exe �����İ���wireshark������ʾΪMalformed Packet. ���п�����wireshark��bug, ���浱ǰcaptured�����ļ�, Ȼ������wireshark�򿪱�����ļ�, ���������ʾMalformed Packet, ��˵��EgpSender.exe�����İ���û�������. Ϊ�˱������˷Ѹ����ʱ������Malformed Packet��ԭ��, �ؽ�wirechar��bug�����������¼�ڴˡ�