#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QTextStream>
#include <iostream>
#include "kourou.h"

static UC_DeviceInfo di;
static Kourou device;
static QString input_path;
static QString output_path;
static int last_error = 0;


void PrintDeviceInfo()
{
    printf("Battery : %2d%%\n", di.battery_capacity);
    printf("AutoRCM : %s\n", di.autoRCM ? "Enabled" : "Disabled");
    printf("Burnt fuses : %d\n", di.burnt_fuses);
    if (di.sdmmc_initialized)
    {
        QString fs;
        if (di.mmc_fs_type == FS_FAT32) fs.append("FAT32");
        else if (di.mmc_fs_type == FS_EXFAT) fs.append("exFAT");
        else fs.append("UNKNOWN");
        printf("SD Filesystem : %s\n", fs.toUtf8().constData());

        qint64 fs_size = 0x200 * (qint64)di.mmc_fs_cl_size * ((qint64)di.mmc_fs_last_cl + 1);
        qint64 fs_free_space = 0x200 * (qint64)di.mmc_fs_cl_size * (qint64)di.mmc_fs_free_cl;

        printf("FS total size : %s\n", QLocale().formattedDataSize(fs_size).toUtf8().constData());
        printf("FS free space : %s\n", QLocale().formattedDataSize(fs_free_space).toUtf8().constData());
    }
    else
    {
        printf("SD Filesystem : N/A (no SD card?)\n");
    }
}

bool SimpleDeviceCommand(UC_CommandType command, void *response, u32 response_size)
{
    UC_Header uc;
    uc.command = command;

    // Send command
    if (device.write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return false;
    // Get response
    if (device.readResponse(response, response_size) != response_size)
        return false;

    return true;
}

void SetAutoRcmOn()
{
    bool success = false;
    if (!SimpleDeviceCommand(SET_AUTORCM_ON, &success, sizeof(success)) || !success)
    {
        printf("Failed to enable autoRCM\n");
        last_error = -1;
        return;
    }
    printf("autoRCM enabled\n");
}

void SetAutoRcmOff()
{
    bool success = false;
    if (!SimpleDeviceCommand(SET_AUTORCM_OFF, &success, sizeof(success)) || !success)
    {
        printf("Failed to disable autoRCM\n");
        last_error = -1;
        return;
    }
    printf("autoRCM disabled\n");
}

void ReadSdFile()
{
    auto error = [&](QString s, int err = -1) {
        printf("%s\n", s.toLocal8Bit().constData());
        last_error = err ? err : -1;
        return;
    };

    if (!input_path.size())
        return error("input path argument (-i) is missing");

    // Read distant file
    std::vector<u8> file;
    int res = device.sdmmc_readFile(input_path.toLocal8Bit().constData(), &file);
    if(res != SUCCESS)
        return error(QString().asprintf("Failed to read file %s", input_path.toLocal8Bit().constData()), res);

    // Write to output
    QFile output(output_path);
    bool output_is_file = false;
    if (output_path.size())
    {
        output.open(QIODevice::WriteOnly);
        if (output.isOpen())
            output_is_file = true;
    }

    if (output_is_file)
    {
        output.write((const char*)file.data(), file.size());
        output.close();
    }
    else
    {
        std::cout << file.data() << std::endl;
    }
}

struct DispatchEntry {
    UC_CommandType command;
    void (*f_ptr)();
    QString command_str;
};

static DispatchEntry dispatchEntries[] = {
    { GET_DEVICE_INFO, &PrintDeviceInfo, "GET_DEVICE_INFO" },
    { SET_AUTORCM_ON, &SetAutoRcmOn, "SET_AUTORCM_ON" },
    { SET_AUTORCM_OFF, &SetAutoRcmOff, "SET_AUTORCM_OFF" },
    { READ_SD_FILE, &ReadSdFile, "READ_SD_FILE" },
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);  
    QStringList args = a.arguments();


    if (argc < 2)
    {
        printf("No argument provided\n");
        return -1;
    }

    // Get & validate command
    QString command = args.at(1);
    DispatchEntry entry;
    entry.command = NONE;
    for (auto cur_entry : dispatchEntries) if (cur_entry.command_str == command)
    {
        entry = cur_entry;
        break;
    }

    if (entry.command == NONE)
    {
        printf("%s is not a valid command\n", command.toLocal8Bit().constData());
        return -1;
    }

    // Get command arguments
    for (int i = 2; i < args.size(); ++i)
    {
        if (args.at(i) == "-i" && i+1 < args.size())
            input_path = args.at(++i);

        if (args.at(i) == "-o" && i+1 < args.size())
            output_path = args.at(++i);
    }

    // Initialize RCM device
    if (!device.initDevice())
    {
        DWORD err = GetLastError();
        printf("Failed to init device. RC = %d\n", err);
        return err ? -int(err) : -1;
    }

    // Launch Ariane if needed
    if (!device.arianeIsReady())
    {
        QFile ariane_file("ariane.bin");
        if (!ariane_file.open(QIODevice::ReadOnly))
        {
            printf("Failed to open ariane.bin\n");
            return -1;
        }

        QByteArray ariane_bin = ariane_file.readAll();
        if (device.hack((u8*)ariane_bin.data(), (u32)ariane_bin.size()) != 0)
        {
            printf("Failed to inject ariane.bin\n");
            return -1;
        }

        if (!device.arianeIsReady_sync())
        {
            printf("Failed to communicate with Ariane\n");
            return -1;
        }
    }

    // Get device info
    if (device.getDeviceInfo(&di) != 0)
    {
        printf("Failed to get device info\n");
        return -1;
    }

    entry.f_ptr();

    return last_error;
}
